#ifndef NODE_OPENNI_CONTEXT_H
#define NODE_OPENNI_CONTEXT_H

#include <v8.h>
#include "node.h"

#include <XnCppWrapper.h>

#include "Joints.h"

using namespace node;
using namespace v8;

#define NODE_OPENNI_MAX_USERS 3


namespace nodeopenni {


  struct JointPos {
    void * context;    
    const char * joint;
    Persistent<String> jointName;
    XnVector3D pos;
    uint user;
    bool firing;
  };  

  struct OpenNIError {
    const char * contextMessage;
    const char * message;
    void * context;
  };

  struct UserEvent {
    const char * type;
    uint userId;
    void * context;
  };

  class Context : ObjectWrap {

    public:
      
      bool running_;
      xn::UserGenerator userGenerator_;

      static void Initialize (v8::Handle<v8::Object> target);
                  Context ();
      virtual     ~Context   ();
      void Poll();
      void JointChangeEvent(void * jointPos);
      void UserEventAsync(const char * eventName, uint userId);
      void EmitUserEvent(UserEvent * userEvent);
      void EmitError(const char * message);
      void AddUser(uint userId);
      void RemoveUser(uint userId);

    private:
      
      xn::Context context_;
      XnCallbackHandle userCallbackHandle_;
      XnCallbackHandle userExitCallbackHandle_;
      XnCallbackHandle poseCallbackHandle_;
      XnCallbackHandle calibrationCallbackHandle_;
      XnCallbackHandle jointConfigurationHandle_;

      JointPos jointPositions_[NODE_OPENNI_MAX_USERS][NODE_OPENNI_JOINT_COUNT] ;
      OpenNIError lastError_;
      
      uv_thread_t event_thread_;
      uv_async_t  uv_async_joint_change_callback_[NODE_OPENNI_MAX_USERS][NODE_OPENNI_JOINT_COUNT];
      uv_async_t  uv_async_error_callback_;
      uv_async_t  uv_async_user_event_callback_;

      bool        users_[NODE_OPENNI_MAX_USERS + 1];
      bool        joints_[NODE_OPENNI_JOINT_COUNT];

      Persistent<String> emitSymbol_;
      Persistent<String> lengthSymbol_;


      static Context*       GetContext       (const Arguments &args);
      Handle<Value>         Init             ();
      Handle<Value>         SetJoints        (Local<Object>  array);
      static Handle<Value>  SetJoints        (const Arguments &args);
      Handle<Value>         Close            ();
      static Handle<Value>  Close            (const Arguments &args);
      static Handle<Value>  New              (const Arguments& args);
      Context (int user_device_number);
      void EmitAsyncError(char * context, XnStatus status);
      void InitPollThread();

  };



}
#endif