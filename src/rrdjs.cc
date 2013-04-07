
#include "rrdjs.h"

using namespace v8;

extern "C" {

	static void init(Handle<Object> target) {
		HandleScope scope;
		NODE_SET_METHOD(target, "create", rrdjs::create);
		NODE_SET_METHOD(target, "info", rrdjs::info);
		NODE_SET_METHOD(target, "update", rrdjs::update);
		NODE_SET_METHOD(target, "fetch", rrdjs::fetch);
	}

	NODE_MODULE(rrdjs_bindings, init)
}
