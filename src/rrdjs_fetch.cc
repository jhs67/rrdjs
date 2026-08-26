
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
		Nan::AsyncResource *async;
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
			b.callback->Call(1, &res, b.async);
		}
		else {
			Local<Object> r = Nan::New<Object>();

			Local<Array> s = Nan::New<Array>(b.sourceCount);
			for (unsigned long i = 0; i < b.sourceCount; ++i)
				Nan::Set(s, i, Nan::New<String>(b.sourceNames[i]).ToLocalChecked());

			unsigned long valueCount = (b.end - b.start) / b.step;
			Local<Array> v = Nan::New<Array>(b.sourceCount * valueCount);
			for (unsigned long i = 0; i < b.sourceCount * valueCount; ++i)
					Nan::Set(v, i, Nan::New<Number>(b.values[i]));

			Nan::Set(r, Nan::New("start").ToLocalChecked(), Nan::New<Number>(double(b.start)));
			Nan::Set(r, Nan::New("end").ToLocalChecked(), Nan::New<Number>(double(b.end)));
			Nan::Set(r, Nan::New("step").ToLocalChecked(), Nan::New<Number>(double(b.step)));
			Nan::Set(r, Nan::New("sources").ToLocalChecked(), s);
			Nan::Set(r, Nan::New("values").ToLocalChecked(), v);

			Local<Value> res[2];
			res[0] = Nan::Null();
			res[1] = r;
			b.callback->Call(2, res, b.async);
		}

		delete b.async;
		delete b.callback;
		delete &b;
	}

	NAN_METHOD(fetch) {
		FetchBoomerang &b = *new FetchBoomerang;
		b.request.data = &b;

		b.filename = *Nan::Utf8String(info[0]);
		b.cf = *Nan::Utf8String(info[1]);
		b.start = Nan::To<uint32_t>(info[2]).FromMaybe(0);
		b.end = Nan::To<uint32_t>(info[3]).FromMaybe(0);
		b.step = Nan::To<uint32_t>(info[4]).FromMaybe(0);
		b.callback = new Nan::Callback(info[5].As<Function>());
		b.async = new Nan::AsyncResource("rrdjs:fetch");

		uv_queue_work(Nan::GetCurrentEventLoop(), &b.request, fetchWorker, fetchAfter);
	}

}
