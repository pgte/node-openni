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

#include <XnCppWrapper.h>

#include "context.h"

namespace nodeopenni {

  //// Utils
  bool
  hasError(XnStatus status) {
    return status != XN_STATUS_OK;
  }

  Handle<Value>
  error(XnStatus status) {
    return ThrowException(Exception::Error(
      String::New(xnGetStatusString(status))));
  }

  //// Callbacks

  User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    printf("New User: %d\n", nId);
    Context * context = (Context) pCookie;
    context.GetPoseDetectionCap().StartPoseDetection(POSE_TO_USE,
                                                             nId);
  }

  ///// Life-cycle

  Context *
  Context::GetContext(const Arguments &args) {
    return ObjectWrap::Unwrap<Context>(args.This());
  }

  Context::Context() {
    //this->context_ = new xn::Context();
    this->context_.AddRef();
    this->userGenerator_.Create(this->context_);

    // Register user callbacks

    this->userGenerator_.RegisterUserCallbacks(
      User_NewUser, User_LostUser,this, h1);
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
    this->context_.Release();
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