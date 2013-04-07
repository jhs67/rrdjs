
#include <string>
#include <vector>
#include <node_buffer.h>
#include "rrdjs.h"

namespace rrdjs {

	using std::string;

	struct InfoBoomerang {
		uv_work_t request;

		string filename;
		Persistent<Function> callback;

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
		HandleScope scope;
		Handle<Object> r = ObjectTemplate::New()->NewInstance();

		while (data) {
			Handle<String> key = String::New(data->key);

			switch (data->type) {
				case RD_I_VAL:
					r->Set(key, Number::New(data->value.u_val));
					break;
				case RD_I_CNT:
					r->Set(key, Number::New(data->value.u_cnt));
					break;
				case RD_I_INT:
					r->Set(key, Number::New(data->value.u_int));
					break;
				case RD_I_STR:
					r->Set(key, String::New(data->value.u_str));
					break;
				case RD_I_BLO: {
					Buffer *b = Buffer::New(data->value.u_blo.size);
					memcpy(node::Buffer::Data(b->handle_), (char*)data->value.u_blo.ptr, data->value.u_blo.size);
					r->Set(key, b->handle_);
					break;
				}
			}
			data = data->next;
		}

		return scope.Close(r);
	}

	void infoAfter(uv_work_t *req, int) {
		HandleScope scope;
		InfoBoomerang &b = *static_cast<InfoBoomerang*>(req->data);

		Handle<Value> res[] = {
			b.data == 0 ? Exception::Error(String::New(b.error.c_str())) : Handle<Value>(Null()),
			b.data == 0 ? Handle<Value>(Null()) : rrdInfoToObject(b.data),
		};

		b.callback->Call(Context::GetCurrent()->Global(), b.data == 0 ? 1 : 2, res);
		delete &b;
	}

	Handle<Value> info(const Arguments &args) {
		HandleScope scope;

		InfoBoomerang &b = *new InfoBoomerang;
		b.request.data = &b;

		b.filename = *String::Utf8Value(args[0]->ToString());
		b.callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));

		uv_queue_work(uv_default_loop(), &b.request, infoWorker, infoAfter);

		return Undefined();
	}

}
