
#include "rrdjs.h"

using namespace v8;

extern "C" {

	static void init(Handle<Object> target) {
		target->Set(NanNew<String>("create"), NanNew<FunctionTemplate>(rrdjs::create)->GetFunction());
		target->Set(NanNew<String>("info"), NanNew<FunctionTemplate>(rrdjs::info)->GetFunction());
		target->Set(NanNew<String>("update"), NanNew<FunctionTemplate>(rrdjs::update)->GetFunction());
		target->Set(NanNew<String>("fetch"), NanNew<FunctionTemplate>(rrdjs::fetch)->GetFunction());
	}

	NODE_MODULE(rrdjs_bindings, init)
}
