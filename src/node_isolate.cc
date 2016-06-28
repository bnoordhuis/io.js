#include "node.h"
#include "node_mutex.h"

#include "base-object.h"
#include "base-object-inl.h"
#include "env.h"
#include "env-inl.h"
#include "util.h"
#include "util-inl.h"
#include "uv.h"

#include <functional>
#include <memory>
#include <vector>

#ifdef NODE_ENABLE_VTUNE_PROFILING
#include "../deps/v8/src/third_party/vtune/v8-vtune.h"
#endif  // NODE_ENABLE_VTUNE_PROFILING

namespace node {

class Context;
class ContextWrap;
class EnvironmentWrap;
class EventLoopWrap;
class Isolate;
class IsolateDataWrap;
class IsolateWrap;
class Script;
class ScriptWrap;

struct Isolate {
  explicit Isolate(v8::Isolate* v8_isolate) : v8_isolate(v8_isolate) {}
  ~Isolate() { v8_isolate->Dispose(); }

  v8::Isolate* const v8_isolate;

  DISALLOW_COPY_AND_ASSIGN(Isolate);
};

template <typename T>
class PersistentValue {
 public:
  PersistentValue(const std::shared_ptr<Isolate>& isolate_ref,
                  v8::Local<T> value)
      : isolate_ref(isolate_ref)
      , persistent_value(isolate_ref->v8_isolate, value) {}

  v8::MaybeLocal<T> maybe_value(const v8::HandleScope& handle_scope) const {
    if (persistent_value.IsEmpty()) return v8::MaybeLocal<T>();
    auto v8_isolate = isolate_ref->v8_isolate;
    CHECK_EQ(v8_isolate, handle_scope.GetIsolate());
    return PersistentToLocal(v8_isolate, persistent_value);
  }

  const std::shared_ptr<Isolate> isolate_ref;
  const v8::Persistent<T> persistent_value;

  DISALLOW_COPY_AND_ASSIGN(PersistentValue);
};

struct Context : public PersistentValue<v8::Context> {
  Context(const std::shared_ptr<Isolate>& isolate_ref,
          v8::Local<v8::Context> value)
      : PersistentValue(isolate_ref, value) {}

  ~Context() {
    isolate_ref->v8_isolate->ContextDisposedNotification();
  }
};

struct Script : public PersistentValue<v8::Script> {
  Script(const std::shared_ptr<Context>& context_ref,
         v8::Local<v8::Script> value)
      : PersistentValue(context_ref->isolate_ref, value)
      , context_ref(context_ref) {}

  const std::shared_ptr<Context> context_ref;
};

// TODO(bnoordhuis) Clean up RefCountedWrap instances when the owning isolate
// is disposed of.  Failing to run the destructors results in resource leaks.
template <typename T>
class RefCountedWrap {
 public:
  RefCountedWrap(v8::Isolate* isolate, v8::Local<v8::Object> holder,
                 const std::shared_ptr<T>& ref)
      : holder_(isolate, holder), ref_(ref) {
    Wrap<RefCountedWrap>(holder, this);
    holder_.SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);
  }

  virtual ~RefCountedWrap() = default;

  std::shared_ptr<T> ref() const { return ref_; }

 private:
  static void WeakCallback(const v8::WeakCallbackInfo<RefCountedWrap>& info) {
    delete info.GetParameter();
  }

  v8::Persistent<v8::Object> holder_;
  const std::shared_ptr<T> ref_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedWrap);
};

class ContextWrap : public RefCountedWrap<Context> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  inline static void CreateProcessObject(
      const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  ContextWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
              const std::shared_ptr<Context>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}
};

class EnvironmentWrap : public RefCountedWrap<Environment> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  EnvironmentWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
                  const std::shared_ptr<Environment>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}
};

class EventLoopWrap : public RefCountedWrap<uv_loop_t> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  EventLoopWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
                const std::shared_ptr<uv_loop_t>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}
};

class IsolateWrap : public RefCountedWrap<Isolate> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  IsolateWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
              const std::shared_ptr<Isolate>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}
};

class IsolateDataWrap : public RefCountedWrap<IsolateData> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  IsolateDataWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
                  const std::shared_ptr<IsolateData>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}
};

class ScriptWrap : public RefCountedWrap<Script> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  inline static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  ScriptWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
             const std::shared_ptr<Script>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}
};

class JoinableThread {
 public:
  inline JoinableThread() {}
#if 0
  inline explicit JoinableThread(
      const std::shared_ptr<IsolateData>& isolate_data_ref)
      : isolate_data_ref(isolate_data_ref) {}
#endif

 private:
  inline void Run();

  //const std::shared_ptr<IsolateData> isolate_data_ref;
  uv_thread_t thread_;

  DISALLOW_COPY_AND_ASSIGN(JoinableThread);
};

class JoinableThreadWrap : public RefCountedWrap<JoinableThread> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  JoinableThreadWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
                     const std::shared_ptr<JoinableThread>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}

  v8::Persistent<v8::Function> callback_function_;

  DISALLOW_COPY_AND_ASSIGN(JoinableThreadWrap);
};

void ContextWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  if (!env->isolate_constructor_template()->HasInstance(args[0])) {
    return env->ThrowError("Isolate expected.");
  }
  auto isolate_ref = Unwrap<IsolateWrap>(args[0].As<v8::Object>())->ref();
  std::shared_ptr<Context> context_ref;
  {
    auto v8_isolate = isolate_ref->v8_isolate;
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    auto v8_context = v8::Context::New(v8_isolate);
    if (!v8_context.IsEmpty()) {
      context_ref = std::make_shared<Context>(isolate_ref, v8_context);
    }
  }
  if (!context_ref) {
    return env->ThrowError("v8::Context::New() failed.");
  }
  new ContextWrap(args.GetIsolate(), args.Holder(), context_ref);
}

void ContextWrap::CreateProcessObject(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto outer_env = Environment::GetCurrent(args);
  if (!outer_env->environment_constructor_template()->HasInstance(args[0])) {
    return outer_env->ThrowError("Environment expected.");
  }
  auto context_ref = Unwrap<ContextWrap>(args.Holder())->ref();
  auto environment_ref =
      Unwrap<EnvironmentWrap>(args[0].As<v8::Object>())->ref();

  const char* errmsg = nullptr;
  do {
    auto inner_env = environment_ref.get();
    auto v8_isolate = context_ref->isolate_ref->v8_isolate;
    if (v8_isolate != inner_env->isolate()) {
      errmsg = "Environment is owned by a different isolate.";
      break;
    }
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    auto maybe_v8_context = context_ref->maybe_value(handle_scope);
    auto v8_context = maybe_v8_context.ToLocalChecked();
    v8::Context::Scope context_scope(v8_context);
    if (v8_context != inner_env->context()) {
      errmsg = "Environment is owned by a different context.";
      break;
    }
    static const char* const argv[] = { "node", nullptr };
    static const char* const exec_argv[] = { "node", nullptr };
    const bool v8_is_profiling = false;
    inner_env->Start(arraysize(argv) - 1, argv,
                     arraysize(exec_argv) - 1, exec_argv,
                     v8_is_profiling);
    Environment::AsyncCallbackScope callback_scope(inner_env);
    LoadEnvironment(inner_env);
  } while (false);

  if (errmsg != nullptr) {
    return outer_env->ThrowError(errmsg);
  }
}

void EnvironmentWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  if (!env->isolate_data_constructor_template()->HasInstance(args[0])) {
    return env->ThrowError("IsolateData expected.");
  }
  if (!env->context_constructor_template()->HasInstance(args[1])) {
    return env->ThrowError("Context expected.");
  }
  auto isolate_data_ref =
      Unwrap<IsolateDataWrap>(args[0].As<v8::Object>())->ref();
  auto context_ref = Unwrap<ContextWrap>(args[1].As<v8::Object>())->ref();
  std::shared_ptr<Environment> environment_ref;
  if (auto v8_isolate = context_ref->isolate_ref->v8_isolate) {
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    auto maybe_v8_context = context_ref->maybe_value(handle_scope);
    auto v8_context = maybe_v8_context.ToLocalChecked();
    v8::Context::Scope context_scope(v8_context);
    environment_ref =
        std::make_shared<Environment>(isolate_data_ref.get(), v8_context);
  }
  new EnvironmentWrap(args.GetIsolate(), args.Holder(), environment_ref);
}

void EventLoopWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  uv_loop_t* event_loop = new uv_loop_t();
  if (int err = uv_loop_init(event_loop)) {
    delete event_loop;
    char errmsg[256];
    snprintf(errmsg, sizeof(errmsg), "uv_loop_init: %s\n", uv_strerror(err));
    return env->ThrowError(errmsg);
  }
  auto deleter = [] (uv_loop_t* event_loop) {
    CHECK_EQ(0, uv_loop_close(event_loop));
    delete event_loop;
  };
  std::shared_ptr<uv_loop_t> event_loop_ref(event_loop, deleter);
  new EventLoopWrap(args.GetIsolate(), args.Holder(), event_loop_ref);
}

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

void IsolateWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = &array_buffer_allocator;
#ifdef NODE_ENABLE_VTUNE_PROFILING
  params.code_event_handler = vTune::GetVtuneCodeEventHandler();
#endif
  auto v8_isolate = v8::Isolate::New(params);
  if (v8_isolate == nullptr) {
    return env->ThrowError("v8::Isolate::New() failed.");
  }
  auto isolate_ref = std::make_shared<Isolate>(v8_isolate);
  new IsolateWrap(args.GetIsolate(), args.Holder(), isolate_ref);
}

void IsolateDataWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  if (!env->isolate_constructor_template()->HasInstance(args[0])) {
    return env->ThrowError("Isolate expected.");
  }
  if (!env->event_loop_constructor_template()->HasInstance(args[1])) {
    return env->ThrowError("EventLoop expected.");
  }
  auto isolate_ref = Unwrap<IsolateWrap>(args[0].As<v8::Object>())->ref();
  auto event_loop_ref = Unwrap<EventLoopWrap>(args[1].As<v8::Object>())->ref();
  std::shared_ptr<IsolateData> isolate_data_ref;
  if (auto v8_isolate = isolate_ref->v8_isolate) {
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    // FIXME(bnoordhuis) Should take shared_ptr instances.
    isolate_data_ref =
        std::make_shared<IsolateData>(v8_isolate, event_loop_ref.get());
  }
  new IsolateDataWrap(args.GetIsolate(), args.Holder(), isolate_data_ref);
}

void JoinableThread::Run() {
}

void JoinableThreadWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  if (!env->isolate_data_constructor_template()->HasInstance(args[0])) {
    return env->ThrowError("IsolateData expected.");
  }
  if (!args[1]->IsFunction()) {
    return env->ThrowError("Function expected.");
  }
#if 0
  auto isolate_data_ref =
      Unwrap<IsolateDataWrap>(args[0].As<v8::Object>())->ref();
  auto callback_function = args[1].As<v8::Function>();
  auto self = std::make_shared<JoinableThread>();
  auto trampoline = [] (void* arg) {
    static_cast<JoinableThread*>(arg)->Run();
  };
  if (int err = uv_thread_create(&self->thread_, trampoline, self)) {
    char errmsg[256];
    snprintf(errmsg, sizeof(errmsg), "uv_thread_create: %s\n",
             uv_strerror(err));
    return env->ThrowError(errmsg);
  }
#endif
  auto joinable_thread_ref = std::make_shared<JoinableThread>();
  new JoinableThreadWrap(args.GetIsolate(), args.Holder(), joinable_thread_ref);
}

void ScriptWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  if (!env->context_constructor_template()->HasInstance(args[0])) {
    return env->ThrowError("Context expected.");
  }
  if (!args[1]->IsString()) {
    return env->ThrowError("String expected.");
  }
  if (!args[2]->IsString()) {
    return env->ThrowError("String expected.");
  }
  auto context_ref = Unwrap<ContextWrap>(args[0].As<v8::Object>())->ref();
  node::Utf8Value script_name(args.GetIsolate(), args[1]);
  node::Utf8Value script_source(args.GetIsolate(), args[2]);
  std::shared_ptr<Script> script_ref;
  if (auto v8_isolate = context_ref->isolate_ref->v8_isolate) {
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    auto maybe_v8_context = context_ref->maybe_value(handle_scope);
    auto v8_context = maybe_v8_context.ToLocalChecked();
    v8::Context::Scope context_scope(v8_context);
    v8::Local<v8::String> script_source_string =
        v8::String::NewFromUtf8(v8_isolate, *script_source,
                                v8::NewStringType::kNormal,
                                script_source.length()).ToLocalChecked();
    v8::Local<v8::String> script_name_string =
        v8::String::NewFromUtf8(v8_isolate, *script_name,
                                v8::NewStringType::kNormal,
                                script_name.length()).ToLocalChecked();
    v8::ScriptOrigin script_origin(script_name_string);
    v8::ScriptCompiler::Source source(script_source_string, script_origin);
    auto maybe_v8_unbound_script =
        v8::ScriptCompiler::CompileUnboundScript(v8_isolate, &source);
    v8::Local<v8::UnboundScript> v8_unbound_script;
    if (!maybe_v8_unbound_script.ToLocal(&v8_unbound_script))
      return;  // Exception pending.
    auto v8_script = v8_unbound_script->BindToCurrentContext();
    CHECK(!v8_script.IsEmpty());
    script_ref = std::make_shared<Script>(context_ref, v8_script);
  }
  new ScriptWrap(args.GetIsolate(), args.Holder(), script_ref);
}

void ScriptWrap::Run(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto script_ref = Unwrap<ScriptWrap>(args.Holder())->ref();
  std::string exception_string;
  if (auto v8_isolate = script_ref->isolate_ref->v8_isolate) {
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    auto maybe_v8_context =
        script_ref->context_ref->maybe_value(handle_scope);
    auto v8_context = maybe_v8_context.ToLocalChecked();
    v8::Context::Scope context_scope(v8_context);
    auto maybe_v8_script = script_ref->maybe_value(handle_scope);
    auto v8_script = maybe_v8_script.ToLocalChecked();
    v8::TryCatch try_catch(v8_isolate);
    auto maybe_result = v8_script->Run(v8_context);
    // The result belongs to a different isolate and context
    // and cannot be allowed to escape to the caller.
    if (try_catch.HasCaught()) {
      CHECK(maybe_result.IsEmpty());  // Silence unused variable warning.
      v8::String::Utf8Value exception(try_catch.Exception());
      exception_string.assign(*exception, exception.length());
    }
  }
  if (!exception_string.empty()) {
    auto string =
        v8::String::NewFromUtf8(args.GetIsolate(), exception_string.data(),
                                v8::NewStringType::kNormal,
                                exception_string.size()).ToLocalChecked();
    args.GetReturnValue().Set(string);
  }
}

struct Thread {
 public:
  inline static
  std::shared_ptr<Thread> New(uv_loop_t* const event_loop,
                              const std::function<void()>& work_cb,
                              const std::function<void()>& done_cb,
                              char (*errmsg)[256]);

 private:
  inline Thread(const std::function<void()>& work_cb,
                const std::function<void()>& done_cb)
      : work_cb(work_cb), done_cb(done_cb) {}

  uv_work_t work_req_;
  const std::function<void()> work_cb;
  const std::function<void()> done_cb;
  std::shared_ptr<Thread> self_ref;

  DISALLOW_COPY_AND_ASSIGN(Thread);
};

class ThreadWrap : public RefCountedWrap<Thread> {
 public:
  inline static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  ThreadWrap(v8::Isolate* v8_isolate, v8::Local<v8::Object> holder,
             const std::shared_ptr<Thread>& ref)
      : RefCountedWrap(v8_isolate, holder, ref) {}

  v8::Persistent<v8::Function> callback_function;
};

std::shared_ptr<Thread> Thread::New(uv_loop_t* const event_loop,
                                    const std::function<void()>& work_cb,
                                    const std::function<void()>& done_cb,
                                    char (*errmsg)[256]) {
  auto inner_work_cb = [] (uv_work_t* req) {
    Thread* self = ContainerOf(&Thread::work_req_, req);
    self->work_cb();
  };
  auto inner_done_cb = [] (uv_work_t* req, int status) {
    CHECK_EQ(0, status);
    Thread* self = ContainerOf(&Thread::work_req_, req);
    self->done_cb();
    self->self_ref.reset();  // Break strong reference.
  };
  auto self_ref = std::shared_ptr<Thread>(new Thread(work_cb, done_cb));
  if (int err = uv_queue_work(event_loop, &self_ref->work_req_,
                              inner_work_cb, inner_done_cb)) {
    snprintf(*errmsg, sizeof(*errmsg), "uv_queue_work: %s\n", uv_strerror(err));
    return std::shared_ptr<Thread>();
  }
  // Retain strong reference while thread is in flight.
  self_ref->self_ref = self_ref;
  return self_ref;
}

void ThreadWrap::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto env = Environment::GetCurrent(args);
  if (!args.IsConstructCall()) {
    return env->ThrowError("Not a construct call.");
  }
  if (!env->script_constructor_template()->HasInstance(args[0])) {
    return env->ThrowError("Script expected.");
  }
  if (!args[1]->IsFunction()) {
    return env->ThrowError("Function expected.");
  }
  auto script_ref = Unwrap<ScriptWrap>(args[0].As<v8::Object>())->ref();
  auto work_cb = [=] () {
    auto v8_isolate = script_ref->context_ref->isolate_ref->v8_isolate;
    v8::Locker locker(v8_isolate);
    v8::Isolate::Scope isolate_scope(v8_isolate);
    v8::HandleScope handle_scope(v8_isolate);
    auto maybe_v8_context = script_ref->context_ref->maybe_value(handle_scope);
    auto v8_context = maybe_v8_context.ToLocalChecked();
    v8::Context::Scope context_scope(v8_context);
    auto maybe_v8_script = script_ref->maybe_value(handle_scope);
    auto v8_script = maybe_v8_script.ToLocalChecked();
    auto maybe_value = v8_script->Run();
    // Exception pending if maybe_value.IsEmpty() is true.
    (void) &maybe_value;
  };
  auto v8_persistent_context_ref =
      std::make_shared<v8::Persistent<v8::Context>>(
          env->isolate(),
          env->isolate()->GetCurrentContext());
  auto v8_persistent_function_ref =
      std::make_shared<v8::Persistent<v8::Function>>(
          env->isolate(),
          args[1].As<v8::Function>());
  auto done_cb = [=] () {
    v8::Locker locker(env->isolate());
    v8::Isolate::Scope isolate_scope(env->isolate());
    v8::HandleScope handle_scope(env->isolate());
    auto v8_context =
        PersistentToLocal(env->isolate(), *v8_persistent_context_ref);
    v8::Context::Scope context_scope(v8_context);
    auto function =
        PersistentToLocal(env->isolate(), *v8_persistent_function_ref);
    v8::Local<v8::Value> argv[] = { v8::Undefined(env->isolate()) };
    auto maybe_value =
        node::MakeCallback(env, v8::Undefined(env->isolate()), function,
                           arraysize(argv), argv);
    // Exception pending if maybe_value.IsEmpty() is true.
    (void) &maybe_value;
  };
  char errmsg[256];
  auto thread_ref = Thread::New(env->event_loop(), work_cb, done_cb, &errmsg);
  if (!thread_ref) {
    return env->ThrowError(errmsg);
  }
  new ThreadWrap(args.GetIsolate(), args.Holder(), thread_ref);
}

inline void InitializeBinding(v8::Local<v8::Object> target,
                              v8::Local<v8::Value> unused,
                              v8::Local<v8::Context> context) {
  using Setter = void (Environment::*)(v8::Local<v8::FunctionTemplate>);

  auto configure = [] (Environment* env, v8::Local<v8::Object> target,
                       const char* name, v8::FunctionCallback callback,
                       Setter set_constructor_template) {
    auto constructor = env->NewFunctionTemplate(callback);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);

    auto constructor_name = OneByteString(env->isolate(), name);
    constructor->SetClassName(constructor_name);

    if (callback == ContextWrap::New) {
      env->SetProtoMethod(constructor, "createProcessObject",
                          ContextWrap::CreateProcessObject);
    } else if (callback == ScriptWrap::New) {
      env->SetProtoMethod(constructor, "run", ScriptWrap::Run);
    }

    auto function = constructor->GetFunction();
    CHECK(target->Set(env->context(), constructor_name, function).FromJust());

    if (set_constructor_template != nullptr) {
      (env->*set_constructor_template)(constructor);
    }
  };

  auto env = Environment::GetCurrent(context);
  configure(env, target, "Context", ContextWrap::New,
            &Environment::set_context_constructor_template);
  configure(env, target, "Environment", EnvironmentWrap::New,
            &Environment::set_environment_constructor_template);
  configure(env, target, "EventLoop", EventLoopWrap::New,
            &Environment::set_event_loop_constructor_template);
  configure(env, target, "Isolate", IsolateWrap::New,
            &Environment::set_isolate_constructor_template);
  configure(env, target, "IsolateData", IsolateDataWrap::New,
            &Environment::set_isolate_data_constructor_template);
  configure(env, target, "JoinableThread", JoinableThreadWrap::New,
            &Environment::set_joinable_thread_constructor_template);
  configure(env, target, "Script", ScriptWrap::New,
            &Environment::set_script_constructor_template);
  configure(env, target, "Thread", ThreadWrap::New, nullptr);
}

}  // namespace node

NODE_MODULE_CONTEXT_AWARE_BUILTIN(isolate, node::InitializeBinding)
