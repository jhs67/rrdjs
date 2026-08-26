
#include "rrdjs.h"

using namespace v8;

NAN_MODULE_INIT(init) {
	Nan::SetMethod(target, "create", rrdjs::create);
	Nan::SetMethod(target, "info", rrdjs::info);
	Nan::SetMethod(target, "update", rrdjs::update);
	Nan::SetMethod(target, "fetch", rrdjs::fetch);
}

NAN_MODULE_WORKER_ENABLED(rrdjs_bindings, init)
