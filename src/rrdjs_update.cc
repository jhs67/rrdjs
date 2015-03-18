
#include <string>
#include <vector>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;
	using std::vector;

	struct UpdateBoomerang {
		uv_work_t request;

		string filename;
		string templte;
		vector<string> args;
		NanCallback *callback;

		int status;
		string error;
	};

	void updateWorker(uv_work_t *req) {
		UpdateBoomerang &b = *static_cast<UpdateBoomerang*>(req->data);

		vector<const char*> args(b.args.size());
		for (size_t i = 0; i < args.size(); ++i) args[i] = b.args[i].c_str();
		b.status = rrd_update_r(b.filename.c_str(), b.templte.c_str(), args.size(), &args[0]);
		if (b.status < 0) b.error = rrd_get_error();
		rrd_clear_error();
	}

	void updateAfter(uv_work_t *req, int) {
		NanScope();
		UpdateBoomerang &b = *static_cast<UpdateBoomerang*>(req->data);
		Handle<Value> res[] = { b.status < 0 ? Exception::Error(NanNew<String>(b.error.c_str())) : Handle<Value>(NanNull()) };
		b.callback->Call(1, res);
		delete b.callback;
		delete &b;
	}

	NAN_METHOD(update) {
		NanScope();

		UpdateBoomerang &b = *new UpdateBoomerang;
		b.request.data = &b;

		b.filename = *String::Utf8Value(args[0]->ToString());
		b.templte = *String::Utf8Value(args[1]->ToString());

		Local<Array> aa = Local<Array>::Cast(args[2]);
		b.args.resize(aa->Length());
		for (size_t i = 0; i < b.args.size(); ++i)
			b.args[i] = *String::Utf8Value(aa->Get(i));

		b.callback = new NanCallback(args[3].As<Function>());

		uv_queue_work(uv_default_loop(), &b.request, updateWorker, updateAfter);

		NanReturnUndefined();
	}

}
