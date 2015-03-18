
#include <string>
#include <vector>
#include <node_buffer.h>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;

	struct InfoBoomerang {
		uv_work_t request;

		string filename;
		NanCallback *callback;

		rrd_info_t *data;
		string error;
	};

	void infoWorker(uv_work_t *req) {
		InfoBoomerang &b = *static_cast<InfoBoomerang*>(req->data);

		b.data = rrd_info_r(const_cast<char*>(b.filename.c_str()));
		if (b.data == 0) b.error = rrd_get_error();
		rrd_clear_error();
	}

	Handle<Value> rrdInfoToObject(rrd_info_t *data) {
		Handle<Object> r = ObjectTemplate::New()->NewInstance();

		while (data) {
			Handle<String> key = NanNew<String>(data->key);

			switch (data->type) {
				case RD_I_VAL:
					r->Set(key, NanNew<Number>(data->value.u_val));
					break;
				case RD_I_CNT:
					r->Set(key, NanNew<Number>(data->value.u_cnt));
					break;
				case RD_I_INT:
					r->Set(key, NanNew<Number>(data->value.u_int));
					break;
				case RD_I_STR:
					r->Set(key, NanNew<String>(data->value.u_str));
					break;
				case RD_I_BLO: {
					Local<Object> b = NanNewBufferHandle(data->value.u_blo.size);
					memcpy(node::Buffer::Data(b), (char*)data->value.u_blo.ptr, data->value.u_blo.size);
					r->Set(key, b);
					break;
				}
			}
			data = data->next;
		}

		return r;
	}

	void infoAfter(uv_work_t *req, int) {
		NanScope();
		InfoBoomerang &b = *static_cast<InfoBoomerang*>(req->data);

		Handle<Value> res[] = {
			b.data == 0 ? Exception::Error(NanNew<String>(b.error.c_str())) : NanNew<Value>(NanNull()),
			b.data == 0 ? NanNew<Value>(NanNull()) : rrdInfoToObject(b.data),
		};

		b.callback->Call(b.data == 0 ? 1 : 2, res);
		delete b.callback;
		delete &b;
	}

	NAN_METHOD(info) {
		NanScope();

		InfoBoomerang &b = *new InfoBoomerang;
		b.request.data = &b;

		b.filename = *String::Utf8Value(args[0]->ToString());
		b.callback = new NanCallback(args[1].As<Function>());

		uv_queue_work(uv_default_loop(), &b.request, infoWorker, infoAfter);

		NanReturnUndefined();
	}

}
