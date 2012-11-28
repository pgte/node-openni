#define BUILDING_NODE_EXTENSION

#include <v8.h>
#include "node.h"
#include "node_buffer.h"
#include "uv.h"

#include <stdio.h>
#include <stdexcept>
#include <string>

using namespace node;
using namespace v8;

#include <XnOpenNI.h>

#include "context.h"

namespace nodeopenni {

  Handle<Value>
  check(XnStatus status) {
    if (status == XN_STATUS_OK) return Undefined();

    return ThrowException(Exception::Error(
      String::New(xnGetStatusString(status))));
  }

  Context *
  Context::GetContext(const Arguments &args) {
    return ObjectWrap::Unwrap<Context>(args.This());
  }

  Handle<Value>
  Context::Init() {
    HandleScope scope;
    return check(xnInit(&context_));
  }


  Handle<Value>
  Context::Init(const Arguments& args) {
    HandleScope scope;
    return GetContext(args)->Init();
  }

  Context::Context() {
    this->context_ = NULL;
    xnContextAddRef(this->context_);
  }

  Handle<Value>
  Context::New(const Arguments& args) {
    HandleScope scope;

    assert(args.IsConstructCall());

    Context *context = new Context();
    context->Wrap(args.This());

    return args.This();
  }

  void
  Context::Close() {
    if (this->context_ != NULL) {
      xnContextRelease(this->context_);
    }
  }

  Handle<Value>
  Context::Close(const Arguments& args) {
    HandleScope scope;
    GetContext(args)->Close();
    return Undefined();
  }

  Context::~Context() {
    Close();
  }

  void
  Context::Initialize(v8::Handle<v8::Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "close", Close);
    NODE_SET_PROTOTYPE_METHOD(t, "init", Init);

    target->Set(String::NewSymbol("Context"), t->GetFunction());
  }

  void
  Initialize(Handle<Object> target) {
    Context::Initialize(target);
  }
}

extern "C" void
init(Handle<Object> target)
{
  nodeopenni::Initialize(target);
}