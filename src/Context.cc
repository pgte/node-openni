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
#include "Conf.h"

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

namespace nodeopenni {

  char * xmConf = "";

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

  void
  printError(char * context, XnStatus status)
  {
    const XnChar * statusStr = xnGetStatusString(status);
    printf("Error in context %s: %s\n", context, statusStr);
  }

  /// Pipe an error into the main loop
  void
  Context::EmitAsyncError(char * context, XnStatus status)
  {
    const XnChar * statusStr = xnGetStatusString(status);
    uv_async_t * callback = &this->uv_async_error_callback_;
    this->lastError_.contextMessage = context;
    this->lastError_.message = statusStr;
    callback->data = & this->lastError_;
    uv_async_send(callback);
    printError(context, status);
  };

  ///// Error Callback

  void
  Context::EmitError(const char * error)
  {
    Local<Value> callback_v = handle_->Get(this->emitSymbol_);
    Local<Function> callback = Local<Function>::Cast(callback_v);
    Handle<Value> argv[2] = { String::New("error"),  Exception::Error(String::New(error))};
    callback->Call(handle_, 2, argv);
  }

  static void
  async_error_callback_(uv_async_t *handle, int notUsed)
  {
    OpenNIError * error = (OpenNIError *) handle->data;
    assert(error);
    nodeopenni::Context * context = (nodeopenni::Context * ) error->context;
    assert(context);
    context->EmitError(error->message);
  }

  ///// User Events

  void
  Context::UserEventAsync(const char * eventType, uint userId)
  {
    UserEvent * ev = (UserEvent *) malloc(sizeof(UserEvent));
    ev->type = eventType;
    ev->userId = userId;
    ev->context = this;
    this->uv_async_user_event_callback_.data = ev;
    uv_async_send(&this->uv_async_user_event_callback_);
  }

  static void
  async_user_event_callback_(uv_async_t *handle, int notUsed)
  {
    UserEvent * event = (UserEvent *) handle->data;
    assert(event);
    nodeopenni::Context * context = (nodeopenni::Context * ) event->context;
    assert(context);
    context->EmitUserEvent(event);
    free(event);
  }

  void
  Context::EmitUserEvent(UserEvent * event)
  {
    Local<Value> callback_v = handle_->Get(this->emitSymbol_);
    Local<Function> callback = Local<Function>::Cast(callback_v);
    Handle<Value> argv[2] = { String::New(event->type),  Number::New(event->userId)};
    callback->Call(handle_, 2, argv);
  }


  ///// Joint Change Event Callback

  static void
  async_joint_change_callback_(uv_async_t *handle, int notUsed)
  {
    JointPos * jointPos = (JointPos *) handle->data;
    assert(jointPos);
    ((Context *) jointPos->context)->JointChangeEvent(jointPos);
  }


  void
  Context::JointChangeEvent(void * pos)
  {
    JointPos * jointPos = (JointPos *) pos;

    Local<Value> callback_v =handle_->Get(this->emitSymbol_);
    Local<Function> callback = Local<Function>::Cast(callback_v);
    Handle<Value> argv[5] = { jointPos->jointName, Number::New(jointPos->user), Number::New(jointPos->pos.X), Number::New(jointPos->pos.Y), Number::New(jointPos->pos.Z) };
    callback->Call(handle_, 5, argv);

    jointPos->firing = FALSE;
  }


  ///// Event Loop

  void
  process_event_thread(void *arg)
  {
    Context * context = (Context *) arg;
    while(context->running_) {
      context->Poll();
    }
  }

  void
  Context::InitPollThread()
  {
    uv_thread_create(&this->event_thread_, process_event_thread, this);
  }

  void
  Context::Poll()
  {
    XnStatus status;

    status = this->context_.WaitAndUpdateAll();
    if (hasError(status)) {
      this->EmitAsyncError("calling context.WaitAndUpdateAll", status);
      return;
    }
    XnUserID aUsers[15];
    XnUInt16 nUsers = 15;

    status = this->userGenerator_.GetUsers(aUsers, nUsers);

    if (hasError(status)) {
      this->EmitAsyncError("getting users", status);
      return;
    }

    /// Poll all joint positions for all available users

    JointPos   *jointPos;
    XnSkeletonJointPosition newJointPos;
    uv_async_t * callback;

    for (int i = 0; i < nUsers && i < NODE_OPENNI_MAX_USERS; ++i)
    {
      if (this->users_[aUsers[i]])
      {
        for (int j = 0; j < NODE_OPENNI_JOINT_COUNT; j++) {

          if (! this->joints_[j]) continue;

          // Load old values
          jointPos = &jointPositions_[i][j];

          // Skip if we're already firing this one
          if (jointPos->firing) continue;

          callback = &this->uv_async_joint_change_callback_[i][j];
          callback->data = (void *) jointPos;

          // Skip if the callback is pending delivery
          if (callback->pending) continue;

          // Get new values
          this->userGenerator_.GetSkeletonCap().GetSkeletonJointPosition(
             aUsers[i], joints[j], newJointPos);

          if (hasError(status)) continue;

          // discard unconfident changes
          if (newJointPos.fConfidence < 0.5) continue;

          // Discard 0 values (???)
          // TODO: check if this is really necessary...
          //if (newJointPos.position.X == 0 && newJointPos.position.Y == 0 && newJointPos.position.Z == 0) continue;

          jointPos->pos.X = newJointPos.position.X;
          jointPos->pos.Y = newJointPos.position.Y;
          jointPos->pos.Z = newJointPos.position.Z;

          jointPos->firing = TRUE;
          uv_async_send(callback);

        }
      }
    }
  }

  ///// User Count

  void
  Context::AddUser(uint userId)
  {
    this->users_[userId] = TRUE;
  }

  void
  Context::RemoveUser(uint userId)
  {
    this->users_[userId] = FALSE;
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

    printf("Using XML conf: %s", XMLConf);
    status = this->context_.RunXmlScript(XMLConf);
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

    status = this->userGenerator_.RegisterToUserExit(
      User_UserExit, this, this->userExitCallbackHandle_);
    if (hasError(status)) return error("registering user exit callbacks", status);

    status = this->userGenerator_.GetPoseDetectionCap().RegisterToPoseCallbacks(
      Pose_Detected, NULL, this, this->poseCallbackHandle_);
    if (hasError(status)) return error("registering pose callbacks", status);

    this->userGenerator_.GetSkeletonCap().RegisterCalibrationCallbacks(
      Calibration_Start, Calibration_End, this, this->calibrationCallbackHandle_);
    if (hasError(status)) return error("registering calibration callbacks", status);

    printf("Registered OpenNI callbacks.\n");

    status = this->userGenerator_.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    if (hasError(status)) return error("setting skeleton profile", status);
    printf("Set skeleton profile.\n");

    ////// ---- Initialize Poll Loop

    //// Initialize the async callbacks

    uv_loop_t *loop = uv_default_loop();


    // Initialize the user event callback

    uv_async_init(loop, &uv_async_user_event_callback_, async_user_event_callback_);

    // Initialize the async error callback

    uv_async_init(loop, &this->uv_async_error_callback_, async_error_callback_);

    /// Initialize the async joint callbacks

    for (int i = 0; i < NODE_OPENNI_MAX_USERS; ++i)
    {
      for (int j = 0; j < NODE_OPENNI_JOINT_COUNT; j++) {
        uv_async_init(loop, &this->uv_async_joint_change_callback_[i][j], async_joint_change_callback_);
      }
    }

    // Start the joint pos polling thread
    this->InitPollThread();

    printf("initiated poll thread.\n");

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

    this->emitSymbol_ = NODE_PSYMBOL("emit");
    this->lengthSymbol_ = NODE_PSYMBOL("length");

    /// Initialize all joint positions to 0
    for (int i = 0; i < NODE_OPENNI_MAX_USERS; ++i)
    {
      for (int j = 0; j < NODE_OPENNI_JOINT_COUNT; j++) {
        jointPositions_[i][j].user = i + 1;
        jointPositions_[i][j].pos.X = 0;
        jointPositions_[i][j].pos.Y = 0;
        jointPositions_[i][j].pos.Z = 0;
        jointPositions_[i][j].joint = jointNames[j];
        jointPositions_[i][j].jointName = Persistent<String>::New(String::New(jointNames[j]));
        jointPositions_[i][j].context = this;
        jointPositions_[i][j].firing = FALSE;
      }
    }

    // Initialize user presence
    for (int i = 0; i <= NODE_OPENNI_MAX_USERS; ++i)
    {
      this->users_[i] = FALSE;
    }

    for (int j = 0; j <= NODE_OPENNI_JOINT_COUNT; ++j)
    {
      this->joints_[j] = TRUE;
    }

    // Initialize Error
    this->lastError_.context = this;
    this->lastError_.message = NULL;

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


  ///// SetJoints

  Handle<Value>
  Context::SetJoints(Local<Object>  array)
  {
    uint length = array->Get(this->lengthSymbol_)->Int32Value();
    int jointPos = -1;
    for(uint i = 0; i < length; i ++)
    {
      Local<String> jointName = array->Get(i)->ToString();
      for(uint j = 0; j < NODE_OPENNI_JOINT_COUNT && jointPos < 0; j++) {
        if (jointName->Equals(String::New(jointNames[j]))) {
          jointPos = j;
        }
      }
      if (jointPos >= 0) {
        this->joints_[jointPos] = TRUE;
      }

    }
    return Undefined();
  }

  Handle<Value>
  Context::SetJoints(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() == 1) {
      return GetContext(args)->SetJoints(args[0]->ToObject());
    } else {
      return ThrowException(Exception::RangeError(
          String::New("joints argument must be an array")));
    }

  }


  ///// Close

  Handle<Value>
  Context::Close()
  {
    /// Stop the loop
    this->running_ = false;

    /// Shut down and release context
    this->context_.Shutdown();
    this->context_.Release();

    uv_thread_join(&this->event_thread_);

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

    NODE_SET_PROTOTYPE_METHOD(t, "setJoints", SetJoints);
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

NODE_MODULE(openni, init);

