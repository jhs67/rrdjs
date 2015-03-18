#ifndef RRDJS_RRDJS_H_
#define RRDJS_RRDJS_H_

#include <v8.h>
#include <node.h>
#include <nan.h>
#include "../rrdtool/src/rrd.h"

namespace rrdjs {

	using namespace v8;
	using namespace node;

	NAN_METHOD(create);
	NAN_METHOD(info);
	NAN_METHOD(update);
	NAN_METHOD(fetch);

}

#endif // RRDJS_RRDJS_H_
