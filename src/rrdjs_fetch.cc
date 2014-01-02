
#include <string>
#include <vector>
#include <stdlib.h>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;
	using std::vector;

	struct FetchBoomerang {
		uv_work_t request;

		Persistent<Function> callback;
		string filename;
		long unsigned step;
		time_t start;
		time_t end;
		string cf;

		int status;
		string error;
		char ** sourceNames;
		rrd_value_t * values;
		unsigned long sourceCount;

		FetchBoomerang() : sourceNames(0), values(0), sourceCount(0) {}

		~FetchBoomerang() {
			if (sourceNames) {
				for (unsigned long i = 0; i < sourceCount; ++i) free(sourceNames[i]);
				free(sourceNames);
			}
			if (values) free(values);
		}
	};

	void fetchWorker(uv_work_t *req) {
		FetchBoomerang &b = *static_cast<FetchBoomerang*>(req->data);
		b.status = rrd_fetch_r(b.filename.c_str(), b.cf.c_str(), &b.start, &b.end, &b.step, &b.sourceCount, &b.sourceNames, &b.values);
		if (b.status < 0) b.error = rrd_get_error();
		rrd_clear_error();
	}

	void fetchAfter(uv_work_t *req, int) {
		HandleScope scope;
		FetchBoomerang &b = *static_cast<FetchBoomerang*>(req->data);

		if (b.status < 0) {
			Handle<Value> res = Exception::Error(String::New(b.error.c_str()));
			b.callback->Call(Context::GetCurrent()->Global(), 1, &res);
		}
		else {
			Handle<Object> r = ObjectTemplate::New()->NewInstance();

			Local<Array> s = Array::New(b.sourceCount);
			for (unsigned long i = 0; i < b.sourceCount; ++i)
				s->Set(i, String::New(b.sourceNames[i]));

			int valueCount = (b.end - b.start) / b.step;
			Local<Array> v = Array::New(b.sourceCount * valueCount);
			for (unsigned long i = 0; i < b.sourceCount * valueCount; ++i)
					v->Set(i, Number::New(b.values[i]));

			r->Set(String::New("start"), Number::New(b.start));
			r->Set(String::New("end"), Number::New(b.end));
			r->Set(String::New("step"), Number::New(b.step));
			r->Set(String::New("sources"), s);
			r->Set(String::New("values"), v);

			Handle<Value> res[2];
			res[0] = Null();
			res[1] = r;
			b.callback->Call(Context::GetCurrent()->Global(), 2, res);
		}

		b.callback.Dispose();
		delete &b;
	}

	Handle<Value> fetch(const Arguments &args) {
		HandleScope scope;

		FetchBoomerang &b = *new FetchBoomerang;
		b.request.data = &b;

		b.filename = *String::Utf8Value(args[0]->ToString());
		b.cf = *String::Utf8Value(args[1]->ToString());
		b.start = args[2]->Uint32Value();
		b.end = args[3]->Uint32Value();
		b.step = args[4]->Uint32Value();
		b.callback = Persistent<Function>::New(Local<Function>::Cast(args[5]));

		uv_queue_work(uv_default_loop(), &b.request, fetchWorker, fetchAfter);

		return Undefined();
	}

}
