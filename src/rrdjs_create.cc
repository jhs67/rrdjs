
#include <string>
#include <vector>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;
	using std::vector;

	struct CreateBoomerang {
		uv_work_t request;

		uint32_t step;
		uint32_t start;
		string filename;
		vector<string> args;
		Nan::Callback *callback;
		Nan::AsyncResource *async;

		int status;
		string error;
	};

	void createWorker(uv_work_t *req) {
		CreateBoomerang &b = *static_cast<CreateBoomerang*>(req->data);

		vector<const char*> args(b.args.size());
		for (size_t i = 0; i < args.size(); ++i) args[i] = b.args[i].c_str();
		b.status = rrd_create_r(b.filename.c_str(), b.step, b.start, args.size(), &args[0]);
		if (b.status < 0) b.error = rrd_get_error();
		rrd_clear_error();
	}

	void createAfter(uv_work_t *req, int) {
		Nan::HandleScope scope;
		CreateBoomerang &b = *static_cast<CreateBoomerang*>(req->data);
		Local<Value> res[] = { b.status < 0 ? Nan::Error(b.error.c_str()) : Local<Value>(Nan::Null()) };
		b.callback->Call(1, res, b.async);
		delete b.async;
		delete b.callback;
		delete &b;
	}

	NAN_METHOD(create) {
		CreateBoomerang &b = *new CreateBoomerang;
		b.request.data = &b;

		b.filename = *Nan::Utf8String(info[0]);
		b.step = Nan::To<uint32_t>(info[1]).FromMaybe(0);
		b.start = Nan::To<uint32_t>(info[2]).FromMaybe(0);

		Local<Array> aa = Local<Array>::Cast(info[3]);
		b.args.resize(aa->Length());
		for (size_t i = 0; i < b.args.size(); ++i)
			b.args[i] = *Nan::Utf8String(Nan::Get(aa, i).ToLocalChecked());

		b.callback = new Nan::Callback(info[4].As<Function>());
		b.async = new Nan::AsyncResource("rrdjs:create");

		uv_queue_work(Nan::GetCurrentEventLoop(), &b.request, createWorker, createAfter);
	}

}
