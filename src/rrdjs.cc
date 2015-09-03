
#include "rrdjs.h"

using namespace v8;

extern "C" {

	static void init(Handle<Object> target) {
		Nan::SetMethod(target, "create", rrdjs::create);
		Nan::SetMethod(target, "info", rrdjs::info);
		Nan::SetMethod(target, "update", rrdjs::update);
		Nan::SetMethod(target, "fetch", rrdjs::fetch);
	}

	NODE_MODULE(rrdjs_bindings, init)
}
