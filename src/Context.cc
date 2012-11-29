#define BUILDING_NODE_EXTENSION

#include <v8.h>
#include "node.h"
#include "node_buffer.h"
#include "uv.h"

#include <stdio.h>
#include <strings.h>

using namespace node;
using namespace v8;

#include <XnCppWrapper.h>

#include "Context.h"
#include "Callbacks.h"
#include "Joints.h"

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

namespace nodeopenni {



  //// Utils
  bool
  hasError(XnStatus status)
  {
    return status != XN_STATUS_OK;
  }

  Handle<Value>
  error(char * context, XnStatus status)
  {
    const XnChar * statusStr = xnGetStatusString(status);
    printf("Error in context %s: %s\n", context, statusStr);
    return ThrowException(Exception::Error(
      String::New(statusStr)));
  }

  void printError(char * context, XnStatus status)
  {
    const XnChar * statusStr = xnGetStatusString(status);
    printf("Error in context %s: %s\n", context, statusStr);
  }

  ///// Event Loop

  void
  process_event_thread(void *arg) {
    Context * context = (Context *) arg;
    while(context->running_) {
      context->Poll();
    }
  }

  void
  Context::InitProcessEventThread() {
    uv_thread_create(&this->event_thread_, process_event_thread, this);
  }

  void Context::Poll() {
    XnStatus status;

    status = this->context_.WaitAndUpdateAll();
    if (hasError(status)) printError("calling context.WaitAndUpdateAll", status);
    XnUserID aUsers[15];
    XnUInt16 nUsers = 15;

    status = this->userGenerator_.GetUsers(aUsers, nUsers);
    
    if (hasError(status)) printError("calling context.WaitAndUpdateAll", status);

    XnVector3D position;
    XnVector3D newPosition;
    XnSkeletonJointPosition jointPos;
    XnSkeletonJointPosition newJointPos;


    /// Poll all joint positions for all available users

    for (int i = 0; i < nUsers && i < NODE_OPENNI_MAX_USERS; ++i)
    {
      if (this->userGenerator_.GetSkeletonCap().IsTracking(aUsers[i]))
      {
        for (int j = 0; j < NODE_OPENNI_JOINT_COUNT; j++) {
          
          // Load old values
          jointPos = jointPositions_[i][j];
          position = jointPos.position;
          
          // Get new values
          this->userGenerator_.GetSkeletonCap().GetSkeletonJointPosition(
             aUsers[i], joints[j], newJointPos);

          // discard unconfident changes
          if (newJointPos.fConfidence < 0.5) continue;

          newPosition = newJointPos.position;

          // scan for changes
          if (round(position.X) != round(newPosition.X) && 
              round(position.Y) != round(newPosition.Y) && 
              round(position.Z) != round(newPosition.Z)
            )
          {
            jointPos = newJointPos;

            printf("%s, %d: (%f,%f,%f) [%f]\n", jointNames[j], aUsers[i],
                   jointPos.position.X, jointPos.position.Y, jointPos.position.Z,
                   jointPos.fConfidence);

          }

        }
      }
    }
  }

  ///// Initialization
  Handle<Value>
  Context::Init()
  {
    XnStatus status;

    //this->context_ = new xn::Context();
    status = this->context_.Init();
    if (hasError(status)) return error("initializing", status);

    printf("Initialized.\n");

    status = this->context_.RunXmlScriptFromFile(
      "conf/Conf.xml");
    if (hasError(status)) return error("running XML script", status);
    printf("Ran XML script from file.\n");

    // this->context_.AddRef();
    status = this->userGenerator_.Create(this->context_);
    if (hasError(status)) return error("creating user generator", status);
    printf("Created user generator.\n");

    // Register user callbacks

    status = this->userGenerator_.RegisterUserCallbacks(
      User_NewUser, User_LostUser, this, this->userCallbackHandle_);
    if (hasError(status)) return error("registering user callbacks", status);

    status = this->userGenerator_.GetPoseDetectionCap().RegisterToPoseCallbacks(
      Pose_Detected, NULL, this, this->poseCallbackHandle_);
    if (hasError(status)) return error("registering pose callbacks", status);

    this->userGenerator_.GetSkeletonCap().RegisterCalibrationCallbacks(
      Calibration_Start, Calibration_End, this, this->calibrationCallbackHandle_);
    if (hasError(status)) return error("registering calibration callbacks", status);

    printf("registered callbacks.\n");

    status = this->userGenerator_.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    if (hasError(status)) return error("setting skeleton profile", status);
    printf("Set skeleton profile.\n");

    // Start event loop
    uv_loop_t *loop = uv_default_loop();
    //uv_async_init(loop, &this->uv_async_depth_callback_, async_skel_callback);
    this->InitProcessEventThread();

    printf("initiated process event thread.\n");

    status = this->context_.StartGeneratingAll();
    if (hasError(status)) return error("starting to generate all", status);

    printf("Started generating all.\n");


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

    /// Initialize all joint positions to 0
    for (int i = 0; i < NODE_OPENNI_MAX_USERS; ++i)
    {
      for (int j = 0; j < NODE_OPENNI_JOINT_COUNT; j++) {
        jointPositions_[i][j].position.X = 0;
        jointPositions_[i][j].position.Y = 0;
        jointPositions_[i][j].position.Z = 0;
        jointPositions_[i][j].fConfidence = 0;
      }
    }
    this->running_ = true;
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

  Handle<Value>
  Context::Close()
  {
    /// Stop the loop
    this->running_ = false;
    
    /// Shut down and release context
    this->context_.Shutdown();
    this->context_.Release();
    
    return Undefined();
  }

  Handle<Value>
  Context::Close(const Arguments& args)
  {
    HandleScope scope;
    return GetContext(args)->Close();
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