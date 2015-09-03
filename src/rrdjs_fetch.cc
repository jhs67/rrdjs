
#include <string>
#include <vector>
#include <stdlib.h>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;
	using std::vector;

	struct FetchBoomerang {
		uv_work_t request;

		Nan::Callback *callback;
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
		Nan::HandleScope scope;

		FetchBoomerang &b = *static_cast<FetchBoomerang*>(req->data);

		if (b.status < 0) {
			Local<Value> res = Nan::Error(b.error.c_str());
			b.callback->Call(1, &res);
		}
		else {
			Local<Object> r = ObjectTemplate::New()->NewInstance();

			Local<Array> s = Nan::New<Array>(b.sourceCount);
			for (unsigned long i = 0; i < b.sourceCount; ++i)
				s->Set(i, Nan::New<String>(b.sourceNames[i]).ToLocalChecked());

			int valueCount = (b.end - b.start) / b.step;
			Local<Array> v = Nan::New<Array>(b.sourceCount * valueCount);
			for (unsigned long i = 0; i < b.sourceCount * valueCount; ++i)
					v->Set(i, Nan::New<Number>(b.values[i]));

			Nan::Set(r, Nan::New("start").ToLocalChecked(), Nan::New<Number>(double(b.start)));
			Nan::Set(r, Nan::New("end").ToLocalChecked(), Nan::New<Number>(double(b.end)));
			Nan::Set(r, Nan::New("step").ToLocalChecked(), Nan::New<Number>(double(b.step)));
			Nan::Set(r, Nan::New("sources").ToLocalChecked(), s);
			Nan::Set(r, Nan::New("values").ToLocalChecked(), v);

			Local<Value> res[2];
			res[0] = Nan::Null();
			res[1] = r;
			b.callback->Call(2, res);
		}

		delete b.callback;
		delete &b;
	}

	NAN_METHOD(fetch) {
		FetchBoomerang &b = *new FetchBoomerang;
		b.request.data = &b;

		b.filename = *String::Utf8Value(info[0]->ToString());
		b.cf = *String::Utf8Value(info[1]->ToString());
		b.start = info[2]->Uint32Value();
		b.end = info[3]->Uint32Value();
		b.step = info[4]->Uint32Value();
		b.callback = new Nan::Callback(info[5].As<Function>());

		uv_queue_work(uv_default_loop(), &b.request, fetchWorker, fetchAfter);
	}

}
