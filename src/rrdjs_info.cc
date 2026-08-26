
#include <string>
#include <vector>
#include <node_buffer.h>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;

	struct InfoBoomerang {
		uv_work_t request;

		string filename;
		Nan::Callback *callback;
		Nan::AsyncResource *async;

		rrd_info_t *data;
		string error;
	};

	void infoWorker(uv_work_t *req) {
		InfoBoomerang &b = *static_cast<InfoBoomerang*>(req->data);

		b.data = rrd_info_r(const_cast<char*>(b.filename.c_str()));
		if (b.data == 0) b.error = rrd_get_error();
		rrd_clear_error();
	}

	Local<Value> rrdInfoToObject(rrd_info_t *data) {
		Local<Object> r = Nan::New<Object>();

		while (data) {
			Local<String> key = Nan::New<String>(data->key).ToLocalChecked();

			switch (data->type) {
				case RD_I_VAL:
					Nan::Set(r, key, Nan::New<Number>(data->value.u_val));
					break;
				case RD_I_CNT:
					Nan::Set(r, key, Nan::New<Number>(data->value.u_cnt));
					break;
				case RD_I_INT:
					Nan::Set(r, key, Nan::New<Number>(data->value.u_int));
					break;
				case RD_I_STR:
					Nan::Set(r, key, Nan::New<String>(data->value.u_str).ToLocalChecked());
					break;
				case RD_I_BLO: {
					Local<Object> b = Nan::NewBuffer(data->value.u_blo.size).ToLocalChecked();
					memcpy(node::Buffer::Data(b), (char*)data->value.u_blo.ptr, data->value.u_blo.size);
					Nan::Set(r, key, b);
					break;
				}
			}
			data = data->next;
		}

		return r;
	}

	void infoAfter(uv_work_t *req, int) {
		Nan::HandleScope scope;
		InfoBoomerang &b = *static_cast<InfoBoomerang*>(req->data);

		Local<Value> res[] = {
			b.data == 0 ? Nan::Error(b.error.c_str()) : Local<Value>(Nan::Null()),
			b.data == 0 ? Local<Value>(Nan::Null()) : rrdInfoToObject(b.data),
		};

		b.callback->Call(b.data == 0 ? 1 : 2, res, b.async);
		delete b.async;
		delete b.callback;
		delete &b;
	}

	NAN_METHOD(info) {
		InfoBoomerang &b = *new InfoBoomerang;
		b.request.data = &b;

		b.filename = *Nan::Utf8String(info[0]);
		b.callback = new Nan::Callback(info[1].As<Function>());
		b.async = new Nan::AsyncResource("rrdjs:info");

		uv_queue_work(Nan::GetCurrentEventLoop(), &b.request, infoWorker, infoAfter);
	}

}
