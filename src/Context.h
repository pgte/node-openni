#ifndef NODE_OPENNI_CONTEXT_H
#define NODE_OPENNI_CONTEXT_H

#include <v8.h>
#include "node.h"

#include <XnCppWrapper.h>

using namespace node;
using namespace v8;

namespace nodeopenni {

  class Context : ObjectWrap {

    public:
      bool running_;
      xn::UserGenerator userGenerator_;

      static void Initialize (v8::Handle<v8::Object> target);
                  Context ();
      virtual     ~Context   ();
      void Poll();

    private:
      xn::Context context_;
      XnCallbackHandle userCallbackHandle_;
      XnCallbackHandle poseCallbackHandle_;
      XnCallbackHandle calibrationCallbackHandle_;
      uv_thread_t event_thread_;

      static Context*       GetContext       (const Arguments &args);
      Handle<Value>         Init             ();
      Handle<Value>         Close            ();
      static Handle<Value>  Close            (const Arguments &args);
      static Handle<Value>  New              (const Arguments& args);
      Context (int user_device_number);
      void InitProcessEventThread();

  };  
}
#endif