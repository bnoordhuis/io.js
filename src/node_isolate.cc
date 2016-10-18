#include "node.h"
#include "uv.h"
#include "v8.h"

#include "async-wrap.h"
#include "env.h"
#include "env-inl.h"

#ifdef NODE_ENABLE_VTUNE_PROFILING
#include "../deps/v8/src/third_party/vtune/v8-vtune.h"
#endif  // NODE_ENABLE_VTUNE_PROFILING

#include <algorithm>
#include <functional>
#include <vector>

namespace node {

class CArray {
 public:
  inline CArray() = default;
  // Exception pending when return value is false.
  inline bool Reset(v8::Local<v8::Context> context, v8::Local<v8::Array> array);
  const char* const* elements() const { return &elements_[0]; }
  size_t size() const { return elements_.size() - ok(); }  // Minus sentinel.
  bool ok() const { return !elements_.empty(); }

 private:
  std::vector<char*> elements_;
  std::vector<char> storage_;
  DISALLOW_COPY_AND_ASSIGN(CArray);
};

bool CArray::Reset(v8::Local<v8::Context> context, v8::Local<v8::Array> array) {
  std::vector<size_t> offsets;
  std::vector<char> storage;
  for (size_t i = 0, n = array->Length(); i < n; i += 1) {
    v8::MaybeLocal<v8::Value> maybe_value(array->Get(context, i));
    v8::Local<v8::Value> value;
    if (!maybe_value.ToLocal(&value)) return false;  // Get property failed.
    v8::String::Utf8Value string(value);
    if (*string == nullptr) return false;  // ToString() failed.
    offsets.push_back(storage.size());
    storage.insert(storage.end(), *string, *string + string.length() + 1);
  }
  std::swap(storage_, storage);
  elements_.resize(1 + offsets.size());  // Reserve space for sentinel NULL.
  for (size_t i = 0; i < offsets.size(); i += 1) {
    elements_[i] = &storage_[offsets[i]];
  }
  return true;
}

struct ThreadContext {
  inline ThreadContext() = default;
  int exit_code;
  CArray argv;
  CArray exec_argv;
  uv_thread_t thread;
  uv_async_t async_handle;
  DISALLOW_COPY_AND_ASSIGN(ThreadContext);  // NOLINT(readability/constructors)
};

struct Defer {
  inline explicit Defer(std::function<void()> action) : action(action) {}
  inline ~Defer() { action(); }
  const std::function<void()> action;
  DISALLOW_COPY_AND_ASSIGN(Defer);  // NOLINT(readability/constructors)
};

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)
#define DEFER(action) const Defer CONCAT(defer_, __LINE__) ([&] { action; })

inline void RunInThread(void* arg) {
  ThreadContext* const cx = static_cast<ThreadContext*>(arg);
  DEFER(CHECK_EQ(0, uv_async_send(&cx->async_handle)));

  uv_loop_t event_loop;
  if (int err = uv_loop_init(&event_loop)) {
    fprintf(stderr, "uv_loop_init(): %s\n", uv_strerror(err));
    fflush(stderr);
    return;
  }
  DEFER(CHECK_EQ(0, uv_loop_close(&event_loop)));

  v8::Isolate::CreateParams params;
  ArrayBufferAllocator array_buffer_allocator;
  params.array_buffer_allocator = &array_buffer_allocator;
#ifdef NODE_ENABLE_VTUNE_PROFILING
  params.code_event_handler = vTune::GetVtuneCodeEventHandler();
#endif
  v8::Isolate* const isolate = v8::Isolate::New(params);
  if (isolate == nullptr) {
    fprintf(stderr, "v8::Isolate::New() failed.\n");
    fflush(stderr);
    return;
  }
  DEFER(isolate->Dispose());
  // TODO(bnoordhuis) isolate->GetHeapProfiler()->StartTrackingHeapObjects()
  // TODO(bnoordhuis) isolate->SetAbortOnUncaughtExceptionCallback()

  v8::Locker locker(isolate);
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  IsolateData isolate_data(isolate, &event_loop,
                           array_buffer_allocator.zero_fill_field());

  {
    // We need a new HandleScope here if the ContextDisposedNotification()
    // call after this block is to have any effect.
    v8::HandleScope handle_scope(isolate);
    auto context = v8::Context::New(isolate);
    if (context.IsEmpty()) {
      fprintf(stderr, "v8::Context::New() failed.\n");
      fflush(stderr);
      return;
    }
    v8::Context::Scope context_scope(context);

    Environment env(&isolate_data, context);
    env.Start(cx->argv.size(), cx->argv.elements(),
              cx->exec_argv.size(), cx->exec_argv.elements(),
              /* start_profiler_idle_notifier */ false);
    {
      Environment::AsyncCallbackScope callback_scope(&env);
      LoadEnvironment(&env);
    }

    // TODO(bnoordhuis) env.set_trace_sync_io(trace_sync_io)
    {
      v8::SealHandleScope seal(isolate);
      bool more;
      do {
        v8_platform.PumpMessageLoop(isolate);
        more = uv_run(env.event_loop(), UV_RUN_ONCE);

        if (more == false) {
          v8_platform.PumpMessageLoop(isolate);
          EmitBeforeExit(&env);

          // Emit `beforeExit` if the loop became alive either after emitting
          // event, or after running some callbacks.
          more = uv_loop_alive(env.event_loop());
          if (uv_run(env.event_loop(), UV_RUN_NOWAIT) != 0)
            more = true;
        }
      } while (more == true);
    }
    // TODO(bnoordhuis) env.set_trace_sync_io(false)

    cx->exit_code = EmitExit(&env);
    // TODO(bnoordhuis) RunAtExit(&env)

    // Collect as much garbage as possible before moving to the cleanup phase.
    isolate->ContextDisposedNotification();
    isolate->MemoryPressureNotification(v8::MemoryPressureLevel::kCritical);

    struct PersistentHandleVisitor : public v8::PersistentHandleVisitor {
      virtual void VisitPersistentHandle(v8::Persistent<v8::Value>* value,
                                         uint16_t class_id) {
        // TODO(bnoordhuis) Filter by Environment.  We should not
        // force-kill objects that belong to another Environment.
        if (class_id == BUFFER_ID) {
          buffers.push_back(value);
        } else if (class_id > AsyncWrap::PROVIDER_NONE &&
                   class_id <= AsyncWrap::PROVIDER_ZLIB) {
          async_wrap_handles.push_back(value);
        }
      }
      std::vector<v8::Persistent<v8::Value>*> async_wrap_handles;
      std::vector<v8::Persistent<v8::Value>*> buffers;
    };

    PersistentHandleVisitor persistent_handle_visitor;
    isolate->VisitHandlesWithClassIds(&persistent_handle_visitor);

    {
      v8::HandleScope handle_scope(isolate);
      v8::TryCatch try_catch(isolate);
      for (auto handle : persistent_handle_visitor.async_wrap_handles) {
        auto value = PersistentToLocal(isolate, *handle);
        CHECK(value->IsObject());
        MakeCallback(&env, value.As<v8::Object>(), "close", 0, nullptr);
        CHECK_EQ(false, try_catch.HasCaught());
      }
    }
  }

  auto force_close_cb = [] (uv_handle_t* handle, void* /* arg */) {
    if (!uv_is_closing(handle)) uv_close(handle, nullptr);
  };
  uv_walk(&event_loop, force_close_cb, nullptr);
  CHECK_EQ(0, uv_run(&event_loop, UV_RUN_DEFAULT));
}

inline void OnClose(uv_handle_t* handle) {
  ThreadContext* cx =
      ContainerOf(&ThreadContext::async_handle,
                  reinterpret_cast<uv_async_t*>(handle));
  delete cx;
}

inline void OnEvent(uv_async_t* async_handle) {
  ThreadContext* cx = ContainerOf(&ThreadContext::async_handle, async_handle);
  uv_close(reinterpret_cast<uv_handle_t*>(&cx->async_handle), OnClose);
}

inline void RunInThread(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto env = Environment::GetCurrent(args);
  if (!args[0]->IsArray()) return env->ThrowError("Array expected.");
  if (!args[1]->IsArray()) return env->ThrowError("Array expected.");
  auto context = args.GetIsolate()->GetCurrentContext();
  auto cx = new ThreadContext();
  if (!cx->argv.Reset(context, args[0].As<v8::Array>())) return;
  if (!cx->exec_argv.Reset(context, args[1].As<v8::Array>())) return;
  if (int err = uv_async_init(env->event_loop(), &cx->async_handle, OnEvent)) {
    delete cx;
    return env->ThrowUVException(err, "uv_async_init");
  }
  if (int err = uv_thread_create(&cx->thread, RunInThread, cx)) {
    uv_close(reinterpret_cast<uv_handle_t*>(&cx->async_handle), OnClose);
    return env->ThrowUVException(err, "uv_thread_create");
  }
}

inline void InitializeBinding(v8::Local<v8::Object> target,
                              v8::Local<v8::Value> unused,
                              v8::Local<v8::Context> context) {
  auto env = Environment::GetCurrent(context);
  env->SetMethod(target, "runInThread", RunInThread);
}

}  // namespace node

NODE_MODULE_CONTEXT_AWARE_BUILTIN(isolate, node::InitializeBinding)
