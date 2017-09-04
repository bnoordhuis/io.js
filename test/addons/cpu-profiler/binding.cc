#include "node.h"
#include "v8.h"
#include "v8-profiler.h"

namespace {

void Start(const v8::FunctionCallbackInfo<v8::Value>& args) {
  assert(args[0]->IsString());
  auto title = args[0].As<v8::String>();
  args.GetIsolate()->GetCpuProfiler()->StartProfiling(title);
}

void Stop(const v8::FunctionCallbackInfo<v8::Value>& args) {
  assert(args[0]->IsString());
  auto title = args[0].As<v8::String>();
  args.GetIsolate()->GetCpuProfiler()->StopProfiling(title)->Delete();
}

void Initialize(v8::Local<v8::Object> binding) {
  v8::Isolate* const isolate = binding->GetIsolate();
  binding->Set(v8::String::NewFromUtf8(isolate, "start"),
               v8::FunctionTemplate::New(isolate, Start)->GetFunction());
  binding->Set(v8::String::NewFromUtf8(isolate, "stop"),
               v8::FunctionTemplate::New(isolate, Stop)->GetFunction());
}

NODE_MODULE(test, Initialize)

}  // anonymous namespace
