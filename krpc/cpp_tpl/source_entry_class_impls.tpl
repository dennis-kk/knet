{{@name}}_t::~{{@name}}_t() {
	krpc_destroy(_rpc);
}

{{@name}}_t* {{@name}}_t::instance() {
	if (!_instance) {
		_instance = new {{@name}}_t();
	}
	return _instance;
}

void {{@name}}_t::finalize() {
	if (_instance) {
		delete _instance;
	}
}

int {{@name}}_t::rpc_proc(stream_t* stream) {
	return krpc_proc(_rpc, stream);
}

krpc_t* {{@name}}_t::get_rpc() {
	return _rpc;
}

