#define BUILDING_NODE_EXTENSION

#include <v8.h>
#include "node.h"
#include "node_buffer.h"
#include "uv.h"

#include <stdio.h>

using namespace node;
using namespace v8;

#include <XnCppWrapper.h>

#include "Context.h"
#include "Callbacks.h"

namespace nodeopenni {


  //// Utils
  bool
  hasError(XnStatus status)
  {
    return status != XN_STATUS_OK;
  }

  Handle<Value>
  error(XnStatus status)
  {
    return ThrowException(Exception::Error(
      String::New(xnGetStatusString(status))));
  }

  ///// Initialization
  Handle<Value>
  Context::Init()
  {
    XnStatus status;

    //this->context_ = new xn::Context();
    status = this->context_.Init();
    if (hasError(status)) return error(status);
    // this->context_.AddRef();
    status = this->userGenerator_.Create(this->context_);
    if (hasError(status)) return error(status);

    // Register user callbacks

    status = this->userGenerator_.RegisterUserCallbacks(
      User_NewUser, User_LostUser, this, this->userCallbackHandle_);
    if (hasError(status)) return error(status);

    this->userGenerator_.GetPoseDetectionCap().RegisterToPoseCallbacks(
      Pose_Detected, NULL, this, this->poseCallbackHandle_);

    this->userGenerator_.GetSkeletonCap().RegisterCalibrationCallbacks(
      Calibration_Start, Calibration_End, this, this->calibrationCallbackHandle_);

    return Undefined();
  }

  ///// Life-cycle

  Context *
  Context::GetContext(const Arguments &args)
  {
    return ObjectWrap::Unwrap<Context>(args.This());
  }

  Context::Context()
  {

  }

  Handle<Value>
  Context::New(const Arguments& args)
  {
    HandleScope scope;

    assert(args.IsConstructCall());

    Context *context = new Context();
    context->Wrap(args.This());

    Handle<Value> initRet = context->Init();

    // Check init return for errors
    // An error is everything but undefined
    if (! initRet->IsUndefined()) {
      return initRet;
    }

    return args.This();
  }

  void
  Context::Close()
  {
    this->context_.Release();
  }

  Handle<Value>
  Context::Close(const Arguments& args)
  {
    HandleScope scope;
    GetContext(args)->Close();
    return Undefined();
  }

  Context::~Context()
  {
    Close();
  }

  void
  Context::Initialize(v8::Handle<v8::Object> target)
  {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "close", Close);

    target->Set(String::NewSymbol("Context"), t->GetFunction());
  }

  void
  Initialize(Handle<Object> target)
  {
    Context::Initialize(target);
  }
}

extern "C" void
init(Handle<Object> target)
{
  nodeopenni::Initialize(target);
}