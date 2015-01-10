// Copyright (c) 2014, StrongLoop Inc.
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "node.h"
#include "node_tick_processor.h"  // Generated.
#include "util.h"
#include "util-inl.h"
#include "uv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <signal.h>
#endif

namespace node {

static const char kPrefix[] = "--nologfile_per_isolate --logfile=";
static const char kDirname[] = "/iojs-XXXXXX";
static const char kFilename[] = "/v8.log";
static FILE* stream;
static char* tmpdir;

inline size_t num_natives() {
  return ARRAY_SIZE(natives) - 1;  // Minus sentinel.
}

// FIXME(bnoordhuis) Duplicates os.tmpdir() from lib/os.js.
// This code runs too early to be able to call into the VM.
inline void TempDirectory(char* path, size_t size) {
#ifdef _WIN32
  const char* tmpdir = getenv("TEMP");
  if (tmpdir == nullptr)
    tmpdir = getenv("TMP");
  if (tmpdir != nullptr) {
    snprintf(path, size, "%s", tmpdir);
    return;
  }
  tmpdir = getenv("SystemRoot");
  if (tmpdir == nullptr)
    tmpdir = getenv("windir");
  if (tmpdir == nullptr)
    tmpdir = "";
  snprintf(path, size, "%s\\temp", tmpdir);
#else
  const char* tmpdir = getenv("TMPDIR");
  if (tmpdir == nullptr)
    tmpdir = getenv("TMP");
  if (tmpdir == nullptr)
    tmpdir = getenv("TEMP");
  if (tmpdir == nullptr)
    tmpdir = "/tmp";
  snprintf(path, size, "%s", tmpdir);
#endif
}

inline void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  for (int i = 0; i < args.Length(); i += 1) {
    v8::String::Utf8Value string(args[i]);
    printf("%s\n", *string);
  }
}

inline void Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* const isolate = args.GetIsolate();
  v8::String::Utf8Value filename(args[0]);

  FILE* stream = fopen(*filename, "r");
  if (stream == nullptr) {
    v8::Local<v8::String> message = v8::String::NewFromUtf8(isolate, *filename);
    isolate->ThrowException(v8::Exception::Error(message));
    return;
  }

  const size_t chunk_size = 16 * 1024;
  char* buffer = nullptr;
  size_t size = 0;

  for (;;) {
    if (void* const address = realloc(buffer, size + chunk_size))
      buffer = static_cast<char*>(address);
    else
      break;

    if (const size_t chars_read = fread(buffer + size, 1, chunk_size, stream))
      size += chars_read;
    else
      break;
  }

  // Caveat emptor: the encoding of the file is unspecified but d8 assumes
  // UTF-8 and therefore so do we.
  v8::Local<v8::String> string =
      v8::String::NewFromUtf8(isolate, buffer, v8::String::kNormalString, size);

  free(buffer);

  args.GetReturnValue().Set(string);
}

inline void ReadLine(const v8::FunctionCallbackInfo<v8::Value>& args) {
  char buffer[4096];

  if (stream == nullptr) {
    snprintf(buffer, sizeof(buffer), "%s%s", tmpdir, kFilename);
    stream = fopen(buffer, "r");
  }

  if (stream == nullptr)
    return;

  if (fgets(buffer, sizeof(buffer), stream) == nullptr) {
    fclose(stream);
    stream = nullptr;
    return;
  }

  if (const size_t size = strlen(buffer))
    if (buffer[size - 1] == '\n')
      buffer[size - 1] = '\0';

  // Caveat emptor: the encoding of the file is unspecified but d8 assumes
  // UTF-8 and therefore so do we.
  args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(), buffer));
}

struct Proc {
  uv_process_t process_handle;
  uv_pipe_t stdout_handle;
  int64_t exit_status;
  int term_signal;
  int read_err;
  char* data;
  size_t size;
  size_t used;
};

void OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  Proc* proc = static_cast<Proc*>(handle->data);
  const size_t available = proc->size - proc->used;
  if (available < suggested_size) {
    const size_t new_size = proc->size + suggested_size;
    if (char* new_data = static_cast<char*>(realloc(proc->data, new_size))) {
      proc->data = new_data;
      proc->size = new_size;
    } else {
      proc->read_err = UV_ENOMEM;
      buf->base = nullptr;
      buf->len = 0;
      uv_stop(handle->loop);
      return;
    }
  }
  buf->base = proc->data + proc->used;
  buf->len = suggested_size;
}

void OnRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  Proc* proc = static_cast<Proc*>(handle->data);
  if (nread < 0) {
    proc->read_err = nread;
  } else {
    proc->used += nread;
  }
}

inline void OnExit(uv_process_t* handle, int64_t exit_status, int term_signal) {
  Proc* proc = static_cast<Proc*>(handle->data);
  proc->exit_status = exit_status;
  proc->term_signal = term_signal;
  uv_stop(handle->loop);
}

// FIXME(bnoordhuis) Duplicates a sizable fraction of the functionality
// from src/spawn_sync.cc.
inline void System(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* const isolate = args.GetIsolate();

  uv_process_options_t options;
  memset(&options, 0, sizeof(options));

  v8::String::Utf8Value program_name(args[0]);
  options.file = *program_name;

  options.args = nullptr;
  if (args[1]->IsArray()) {
    v8::Local<v8::Array> params(args[1].As<v8::Array>());
    const int32_t n = params->Length();
    size_t size = (n + 2) * sizeof(char*);  // Program name + terminating NULL.
    union U {
      uintptr_t offset;
      char* pointer;
    };
    U* argv = nullptr;
    for (int32_t i = 1; i <= n; i += 1) {
      v8::String::Utf8Value param(params->Get(i - 1));
      const size_t param_size = param.length() + 1;
      const size_t new_size = size + param_size;
      // Small allocation, not expected to fail.
      CHECK(argv = static_cast<U*>(realloc(argv, new_size)));
      memcpy(reinterpret_cast<char*>(argv) + size, *param, param_size);
      argv[i].offset = size;
      size = new_size;
    }
    // Now turn the offsets in the argv array into pointers.  We can't do that
    // earlier because the argv pointer may change after a realloc() call.
    for (int32_t i = 1; i <= n; i += 1)
      argv[i].pointer = reinterpret_cast<char*>(argv) + argv[i].offset;
    argv[0].pointer = *program_name;
    argv[n + 1].pointer = nullptr;
    options.args = &argv[0].pointer;
  }

  Proc proc;
  memset(&proc, 0, sizeof(proc));
  proc.process_handle.data = &proc;
  proc.stdout_handle.data = &proc;
  proc.data = nullptr;

  uv_loop_t event_loop;
  CHECK_EQ(0, uv_loop_init(&event_loop));
  CHECK_EQ(0, uv_pipe_init(&event_loop, &proc.stdout_handle, 0));

  uv_stdio_container_t stdio[2];  // stdin and stdout
  stdio[0].flags = UV_IGNORE;
  stdio[1].flags = UV_CREATE_PIPE;
  stdio[1].data.stream = reinterpret_cast<uv_stream_t*>(&proc.stdout_handle);

  options.stdio_count = ARRAY_SIZE(stdio);
  options.stdio = stdio;

  const int spawn_err = uv_spawn(&event_loop, &proc.process_handle, &options);
  if (spawn_err == 0) {
    uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(&proc.stdout_handle);
    CHECK_EQ(0, uv_read_start(stream, OnAlloc, OnRead));
    CHECK_EQ(0, uv_run(&event_loop, UV_RUN_DEFAULT));
  }

  uv_close(reinterpret_cast<uv_handle_t*>(&proc.stdout_handle), nullptr);
  uv_close(reinterpret_cast<uv_handle_t*>(&proc.process_handle), nullptr);
  CHECK_EQ(0, uv_run(&event_loop, UV_RUN_DEFAULT));
  CHECK_EQ(0, uv_loop_close(&event_loop));

  if (spawn_err == 0 &&
      proc.read_err == UV_EOF &&
      proc.exit_status == 0 &&
      proc.term_signal == 0) {
    // Caveat emptor: the encoding of stdout is unspecified but d8 assumes
    // UTF-8 and therefore so do we.
    v8::String::NewStringType type = v8::String::kNormalString;
    v8::Local<v8::String> output =
        v8::String::NewFromUtf8(isolate, proc.data, type, proc.used);
    args.GetReturnValue().Set(output);
  } else {
    // It doesn't really matter what we throw, the tick processor doesn't
    // print the error anyway.
    isolate->ThrowException(v8::Null(isolate));
  }

  free(options.args);
  free(proc.data);
}

void ConfigureTickLogging() {
  if (num_natives() == 0)
    return;  // Compiled without tick processor support.

  if (tmpdir != nullptr)
    return;

  const size_t slop = sizeof(kPrefix) + sizeof(kDirname) + sizeof(kFilename);
  char logfile[4096];
  TempDirectory(logfile, sizeof(logfile) - slop);
  strcat(logfile, kDirname);

  uv_fs_t req;
  if (int err = uv_fs_mkdtemp(uv_default_loop(), &req, logfile, nullptr)) {
    fprintf(stderr, "iojs: %s: %s\n", logfile, uv_strerror(err));
  } else {
    const char* path = static_cast<const char*>(req.path);
    snprintf(logfile, sizeof(logfile), "--logfile=%s%s", path, kFilename);
    char program_name[] = "iojs";  // Mandatory but ignored.
    char nologfile_per_isolate[] = "--nologfile_per_isolate";
    char* argv[] = {program_name, nologfile_per_isolate, logfile, nullptr};
    int argc = ARRAY_SIZE(argv) - 1;
    v8::V8::SetFlagsFromCommandLine(&argc, argv, false);
    size_t size = 1 + strlen(path);
    tmpdir = new char[size];
    memcpy(tmpdir, path, size);
  }
  uv_fs_req_cleanup(&req);
}

void RunTickProcessor() {
  if (tmpdir == nullptr)
    return;

#ifndef _WIN32
  // Stop our post-processing from showing up in the CPU profiler output by
  // blocking the SIGPROF signal.  V8 tears down the signal-sending thread
  // but that happens asynchronously and takes long enough that the tick
  // processor scripts show up.  V8::SetFlagsFromCommandLine("--noprof")
  // doesn't work and clobbers the log file besides.
  sigset_t sigmask;
  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGPROF);
  CHECK_EQ(0, pthread_sigmask(SIG_BLOCK, &sigmask, nullptr));
#endif

  char filename[4096];
  snprintf(filename, sizeof(filename), "%s%s", tmpdir, kFilename);

  v8::Isolate* isolate = v8::Isolate::New();
  {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context(v8::Context::New(isolate));
    v8::Context::Scope context_scope(context);

    v8::Local<v8::Array> arguments(v8::Array::New(isolate));
    arguments->Set(0, v8::String::NewFromUtf8(isolate, "--separate-ic"));
    arguments->Set(1, v8::String::NewFromUtf8(isolate, "--unix"));
    arguments->Set(2, v8::String::NewFromUtf8(isolate, filename));

    v8::Local<v8::Object> os_object(v8::Object::New(isolate));
    os_object->Set(v8::String::NewFromUtf8(isolate, "system"),
                   v8::FunctionTemplate::New(isolate, System)->GetFunction());

    v8::Local<v8::Object> global(context->Global());
    global->Set(v8::String::NewFromUtf8(isolate, "arguments"), arguments);
    global->Set(v8::String::NewFromUtf8(isolate, "print"),
                v8::FunctionTemplate::New(isolate, Print)->GetFunction());
    global->Set(v8::String::NewFromUtf8(isolate, "os"), os_object);
    global->Set(v8::String::NewFromUtf8(isolate, "read"),
                v8::FunctionTemplate::New(isolate, Read)->GetFunction());
    global->Set(v8::String::NewFromUtf8(isolate, "readline"),
                v8::FunctionTemplate::New(isolate, ReadLine)->GetFunction());

    for (size_t i = 0; i < num_natives(); i += 1) {
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::String> filename =
          v8::String::NewFromUtf8(isolate, natives[i].name);
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8(isolate,
                                  natives[i].source,
                                  v8::String::kNormalString,
                                  natives[i].source_len);
      v8::TryCatch try_catch;
      v8::Local<v8::Script> script = v8::Script::Compile(source, filename);
      CHECK_EQ(false, script.IsEmpty());
      CHECK_EQ(false, try_catch.HasCaught());
      script->Run();
      if (try_catch.HasCaught()) {
        v8::String::Utf8Value stack_trace_string(try_catch.StackTrace());
        fprintf(stderr, "%s\n", *stack_trace_string);
        break;
      }
    }
  }
  isolate->Dispose();

  uv_fs_t req;
  if (const int err = uv_fs_unlink(uv_default_loop(), &req, filename, NULL))
    fprintf(stderr, "iojs: %s: %s\n", filename, uv_strerror(err));
  uv_fs_req_cleanup(&req);

  if (const int err = uv_fs_rmdir(uv_default_loop(), &req, tmpdir, NULL))
    fprintf(stderr, "iojs: %s: %s\n", tmpdir, uv_strerror(err));
  uv_fs_req_cleanup(&req);

  delete[] tmpdir;
}

}  // namespace node
