#include "node.h"
#include "env.h"
#include "env-inl.h"
#include "util.h"
#include "util-inl.h"
#include "uv.h"

#include <algorithm>
#include <map>
#include <vector>

#ifdef NODE_ENABLE_VTUNE_PROFILING
#include "../deps/v8/src/third_party/vtune/v8-vtune.h"
#endif  // NODE_ENABLE_VTUNE_PROFILING

namespace node {

class IsolateAndContexts;

class Thread {
 public:
  IsolateAndContexts* isolate_and_contexts_ = nullptr;
};

class ThreadWrap {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>&);

 private:
  v8::Persistent<v8::Object> holder_;

  DISALLOW_COPY_AND_ASSIGN(ThreadWrap);
};

class IsolateWrap {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>&);

  inline static void NewContext(const v8::FunctionCallbackInfo<v8::Value>&);
  inline static void CreateAsyncHandle(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  inline static void CreateIsolateData(
      const v8::FunctionCallbackInfo<v8::Value>&);
  inline static void CreateProcessObject(
      const v8::FunctionCallbackInfo<v8::Value>&);
  inline static void RunEventLoop(const v8::FunctionCallbackInfo<v8::Value>&);
  inline static void RunInContext(const v8::FunctionCallbackInfo<v8::Value>&);

  inline IsolateAndContexts* isolate_and_contexts() const;

 private:
  friend void ThreadWrap::New(const v8::FunctionCallbackInfo<v8::Value>&);

  inline IsolateWrap(v8::Isolate* isolate,
                     v8::Local<v8::Object> holder,
                     IsolateAndContexts* isolate_and_contexts);

  inline std::string RunInContext(
      IsolateAndContexts* isolate_and_contexts,
      uint32_t context_id,
      const char* script_name, size_t script_name_size,
      const char* script_source, size_t script_source_size);

  IsolateAndContexts* isolate_and_contexts_ = nullptr;
  v8::Persistent<v8::Object> holder_;

  DISALLOW_COPY_AND_ASSIGN(IsolateWrap);
};

class IsolateAndContexts {
 public:
  using Contexts = std::map<uint32_t, v8::Persistent<v8::Context>*>;
  using Environments = std::map<uint32_t, Environment*>;

  inline explicit IsolateAndContexts(v8::Isolate* isolate);
  inline uint32_t AddContext(v8::Local<v8::Context> context);
  inline bool DisposeContext(uint32_t context_id);

  inline v8::Isolate* isolate() const { return isolate_; }
  inline IsolateData* isolate_data() const { return isolate_data_; }
  inline uv_loop_t* event_loop() const { return event_loop_; }
  inline const Contexts* contexts() const { return &contexts_; }
  inline const Environments* environments() const { return &environments_; }

 private:
  friend void IsolateWrap::CreateIsolateData(
      const v8::FunctionCallbackInfo<v8::Value>&);
  friend void IsolateWrap::CreateProcessObject(
      const v8::FunctionCallbackInfo<v8::Value>&);

  v8::Isolate* isolate_ = nullptr;
  uv_loop_t* event_loop_ = nullptr;  // &this->event_loop_storage_ or nullptr.
  IsolateData* isolate_data_ = nullptr;
  uint32_t context_id_counter_ = 0;
  Contexts contexts_;
  Environments environments_;
  uv_loop_t event_loop_storage_;
  uv_async_t async_handle_;

  DISALLOW_COPY_AND_ASSIGN(IsolateAndContexts);
};

// TODO(bnoordhuis) Probably also useful outside this file.
class CArray {
 public:
  // Have pending JS exception if !ok() after construction.
  CArray(v8::Local<v8::Context> context, v8::Local<v8::Array> array) {
    std::vector<size_t> offsets;
    std::vector<char> storage;
    for (size_t i = 0, n = array->Length(); i < n; i += 1) {
      auto maybe_value = array->Get(context, i);
      if (maybe_value.IsEmpty()) return;  // Get property operation failed.
      v8::String::Utf8Value string(maybe_value.ToLocalChecked());
      if (*string == nullptr) return;  // ToString() operation failed.
      offsets.push_back(storage.size());
      storage.insert(storage.end(), *string, *string + string.length() + 1);
    }
    std::swap(storage_, storage);
    elements_.resize(1 + offsets.size());  // Reserve space for sentinel NULL.
    for (size_t i = 0; i < offsets.size(); i += 1) {
      elements_[i] = &storage_[offsets[i]];
    }
  }

  const char* const* elements() const { return &elements_[0]; }
  size_t size() const { return elements_.size() - ok(); }  // Minus sentinel.
  bool ok() const { return !elements_.empty(); }

 private:
  std::vector<char*> elements_;
  std::vector<char> storage_;
};

// TODO(bnoordhuis) Use node::ArrayBufferAllocator
static struct : public v8::ArrayBuffer::Allocator {
  inline void* Allocate(size_t size) override {
    return calloc(size, 1);
  }
  inline void* AllocateUninitialized(size_t size) override {
    return malloc(size);
  }
  inline void Free(void* data, size_t) override {
    free(data);
  }
} array_buffer_allocator;

IsolateAndContexts::IsolateAndContexts(v8::Isolate* isolate)
    : isolate_(isolate) {}

uint32_t IsolateAndContexts::AddContext(v8::Local<v8::Context> context) {
  auto context_id = ++context_id_counter_;
  auto persistent_context =
      new v8::Persistent<v8::Context>(isolate(), context);
  contexts_.insert(std::make_pair(context_id, persistent_context));
  return context_id;
}

bool IsolateAndContexts::DisposeContext(uint32_t context_id) {
  auto it = contexts_.find(context_id);
  if (it == contexts_.end()) return false;
  delete it->second;
  contexts_.erase(it);
  return true;
}

IsolateWrap::IsolateWrap(v8::Isolate* isolate,
                         v8::Local<v8::Object> holder,
                         IsolateAndContexts* isolate_and_contexts)
    : isolate_and_contexts_(isolate_and_contexts), holder_(isolate, holder) {
  Wrap<IsolateWrap>(holder, this);
}

IsolateAndContexts* IsolateWrap::isolate_and_contexts() const {
  return isolate_and_contexts_;
}

void IsolateWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  v8::Isolate* isolate;
  {
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = &array_buffer_allocator;
#ifdef NODE_ENABLE_VTUNE_PROFILING
    params.code_event_handler = vTune::GetVtuneCodeEventHandler();
#endif
    isolate = v8::Isolate::New(params);
    if (isolate == nullptr) {
      return env->ThrowError("v8::Isolate::New() failed.");
    }
  }
  auto isolate_and_contexts = new IsolateAndContexts(isolate);
  new IsolateWrap(args.GetIsolate(), args.Holder(), isolate_and_contexts);
}

#define THROW_AND_RETURN_IF_NEUTERED(arg)                                     \
  auto env = Environment::GetCurrent(args);                                   \
  if (!env->isolate_wrap_constructor_template()->HasInstance(arg)) {          \
    return env->ThrowError("IsolateWrap expected.");                          \
  }                                                                           \
  auto isolate_wrap = Unwrap<IsolateWrap>(arg.As<v8::Object>());              \
  auto isolate_and_contexts = isolate_wrap->isolate_and_contexts();           \
  if (isolate_and_contexts == nullptr) {                                      \
    return env->ThrowError("IsolateWrap is neutered.");                                   \
  }

void IsolateWrap::NewContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args.Holder());
  uint32_t context_id;
  {
    auto isolate = isolate_and_contexts->isolate();
    v8::Locker isolate_locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    auto context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    context_id = isolate_and_contexts->AddContext(context);
  }
  args.GetReturnValue().Set(context_id);
}

void IsolateWrap::CreateIsolateData(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args.Holder());
  if (isolate_and_contexts->event_loop()) {
    return env->ThrowError("Event loop already initialized.");
  }
  if (isolate_and_contexts->isolate_data()) {
    return env->ThrowError("IsolateData already initialized.");
  }
  uv_loop_t* const event_loop = &isolate_and_contexts->event_loop_storage_;
  if (int err = uv_loop_init(event_loop)) {
    char errmsg[256];
    snprintf(errmsg, sizeof(errmsg), "uv_loop_init: %s\n", uv_strerror(err));
    return env->ThrowError(errmsg);
  }
  {
    auto isolate = isolate_and_contexts->isolate();
    v8::Locker isolate_locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    isolate_and_contexts->event_loop_ = event_loop;
    isolate_and_contexts->isolate_data_ = new IsolateData(isolate, event_loop);
  }
}

void IsolateWrap::CreateProcessObject(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args.Holder());
  if (isolate_and_contexts->event_loop() == nullptr) {
    return env->ThrowError("No event loop.");
  }
  if (isolate_and_contexts->isolate_data() == nullptr) {
    return env->ThrowError("No IsolateData.");
  }
  if (!args[0]->IsUint32()) return env->ThrowError("Number expected.");
  if (!args[1]->IsArray()) return env->ThrowError("Array expected.");
  if (!args[2]->IsArray()) return env->ThrowError("Array expected.");
  uint32_t context_id = args[0]->Uint32Value(env->context()).FromMaybe(0);
  CArray argv(env->context(), args[1].As<v8::Array>());
  CArray exec_argv(env->context(), args[2].As<v8::Array>());
  if (!argv.ok()) return;  // Exception pending.
  if (!exec_argv.ok()) return;  // Exception pending.
  v8::Persistent<v8::Context>* persistent_context;
  {
    auto contexts = isolate_and_contexts->contexts();
    auto it = contexts->find(context_id);
    if (it == contexts->end()) return env->ThrowError("No such context.");
    persistent_context = it->second;
  }
  auto environments = &isolate_and_contexts->environments_;
  if (environments->find(context_id) != environments->end()) {
    return env->ThrowError("Existing environment.");
  }
  Environment* new_env;
  {
    auto isolate = isolate_and_contexts->isolate();
    v8::Locker isolate_locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    auto context = PersistentToLocal(isolate, *persistent_context);
    v8::Context::Scope context_scope(context);
    new_env = new Environment(isolate_and_contexts->isolate_data(), context);
    const bool start_profiler_idle_notifier = false;
    new_env->Start(argv.size(), argv.elements(),
                   exec_argv.size(), exec_argv.elements(),
                   start_profiler_idle_notifier);
    Environment::AsyncCallbackScope callback_scope(new_env);
    LoadEnvironment(new_env);
  }
  environments->insert(std::make_pair(context_id, new_env));
}

void IsolateWrap::CreateAsyncHandle(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args.Holder());
  auto event_loop = isolate_and_contexts->event_loop();
  if (event_loop == nullptr) return env->ThrowError("No event loop.");
}

void IsolateWrap::RunEventLoop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args.Holder());
  auto event_loop = isolate_and_contexts->event_loop();
  if (event_loop == nullptr) return env->ThrowError("No event loop.");
  int result;
  {
    auto isolate = isolate_and_contexts->isolate();
    v8::Locker isolate_locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    // FIXME(bnoordhuis) Highly unsafe when a callback moves the IsolateWrap
    // object to another thread.
    result = uv_run(event_loop, UV_RUN_DEFAULT);
  }
  args.GetReturnValue().Set(result);
}

std::string IsolateWrap::RunInContext(
    IsolateAndContexts* isolate_and_contexts,
    uint32_t context_id,
    const char* script_name, size_t script_name_size,
    const char* script_source, size_t script_source_size) {
  auto contexts = isolate_and_contexts->contexts();
  auto it = contexts->find(context_id);
  if (it == contexts->end()) return "No such context.";
  auto isolate = isolate_and_contexts->isolate();
  v8::Locker isolate_locker(isolate);
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  auto context = PersistentToLocal(isolate, *it->second);
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> script_name_string;
  v8::Local<v8::String> script_source_string;
  v8::Local<v8::UnboundScript> unbound_script;
  {
    auto maybe_value =
        v8::String::NewFromUtf8(isolate, script_name,
                                v8::NewStringType::kNormal, script_name_size);
    if (!maybe_value.ToLocal(&script_name_string)) {
      return "v8::String::NewFromUtf8 failed.";
    }
  }
  {
    auto maybe_value =
        v8::String::NewFromUtf8(isolate, script_source,
                                v8::NewStringType::kNormal, script_source_size);
    if (!maybe_value.ToLocal(&script_source_string)) {
      return "v8::String::NewFromUtf8 failed.";
    }
  }
  {
    v8::ScriptOrigin script_origin(script_name_string);
    v8::ScriptCompiler::Source script_source(
        script_source_string, script_origin);
    // TODO(bnoordhuis) Do something with the compilation exception.
    v8::TryCatch try_catch(isolate);
    auto maybe_value =
        v8::ScriptCompiler::CompileUnboundScript(isolate, &script_source);
    if (!maybe_value.ToLocal(&unbound_script)) {
      v8::String::Utf8Value exception_string(try_catch.Exception());
      return *exception_string;
    }
  }
  auto script = unbound_script->BindToCurrentContext();
  v8::TryCatch try_catch(isolate);
  script->Run(context);
  if (try_catch.HasCaught()) {
    v8::String::Utf8Value exception_string(try_catch.Exception());
    return *exception_string;
  }
  return std::string();
}

void IsolateWrap::RunInContext(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args.Holder());
  if (!args[0]->IsUint32()) return env->ThrowError("Number expected.");
  if (!args[1]->IsString()) return env->ThrowError("String expected.");
  if (!args[2]->IsString()) return env->ThrowError("String expected.");
  uint32_t context_id = args[0]->Uint32Value(env->context()).FromMaybe(0);
  v8::String::Utf8Value script_name(args[1]);
  v8::String::Utf8Value script_source(args[2]);
  std::string error_message =
      isolate_wrap->RunInContext(isolate_and_contexts, context_id,
                                 *script_name, script_name.length(),
                                 *script_source, script_source.length());
  if (!error_message.empty()) return env->ThrowError(error_message.c_str());
}

void ThreadWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  THROW_AND_RETURN_IF_NEUTERED(args[0]);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  //isolate_wrap->isolate_and_contexts_ = nullptr;  // Neuter IsolateWrap.
}

inline void InitializeBinding(v8::Local<v8::Object> target,
                              v8::Local<v8::Value> unused,
                              v8::Local<v8::Context> context) {
  auto env = Environment::GetCurrent(context);

  {
    auto constructor = env->NewFunctionTemplate(IsolateWrap::New);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);

    auto constructor_name = OneByteString(env->isolate(), "IsolateWrap");
    constructor->SetClassName(constructor_name);

    env->SetProtoMethod(constructor, "newContext", IsolateWrap::NewContext);
    env->SetProtoMethod(constructor, "createAsyncHandle",
                        IsolateWrap::CreateAsyncHandle);
    env->SetProtoMethod(constructor, "createIsolateData",
                        IsolateWrap::CreateIsolateData);
    env->SetProtoMethod(constructor, "createProcessObject",
                        IsolateWrap::CreateProcessObject);
    env->SetProtoMethod(constructor, "runEventLoop", IsolateWrap::RunEventLoop);
    env->SetProtoMethod(constructor, "runInContext", IsolateWrap::RunInContext);

    auto function = constructor->GetFunction();
    CHECK(target->Set(context, constructor_name, function).FromJust());

    env->set_isolate_wrap_constructor_template(constructor);
  }

  {
    auto constructor = env->NewFunctionTemplate(IsolateWrap::New);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);

    auto constructor_name = OneByteString(env->isolate(), "ThreadWrap");
    constructor->SetClassName(constructor_name);

    auto function = constructor->GetFunction();
    CHECK(target->Set(context, constructor_name, function).FromJust());
  }
}

}  // namespace node

NODE_MODULE_CONTEXT_AWARE_BUILTIN(isolate, node::InitializeBinding)
