#ifndef NODE_OPENNI_CONTEXT_H
#define NODE_OPENNI_CONTEXT_H

#include <v8.h>
#include "node.h"

#include <XnOpenNI.h>

using namespace node;
using namespace v8;

namespace nodeopenni {


  class Context : ObjectWrap {

    public:
      static void Initialize (v8::Handle<v8::Object> target);
                  Context ();
      virtual     ~Context   ();

    private:
      XnContextPtr context_;

      static Context*       GetContext       (const Arguments &args);
      Handle<Value>         Init             ();
      static Handle<Value>  Init             (const Arguments &args);
      void                  Close            ();
      static Handle<Value>  Close            (const Arguments &args);
      static Handle<Value>  New              (const Arguments& args);
      Context (int user_device_number);

  };  
}
#endif