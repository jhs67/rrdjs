#ifndef RRDJS_RRDJS_H_
#define RRDJS_RRDJS_H_

#include <v8.h>
#include <node.h>
#include "../rrdtool/src/rrd.h"

namespace rrdjs {

	using namespace v8;
	using namespace node;

	Handle<Value> create(const Arguments &args);
	Handle<Value> info(const Arguments &args);
	Handle<Value> update(const Arguments &args);
	Handle<Value> fetch(const Arguments &args);

}

#endif // RRDJS_RRDJS_H_
