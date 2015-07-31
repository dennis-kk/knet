);
	int error = krpc_call(_rpc, stream, {{$rpcid}}, o);
	krpc_object_destroy(o);
	return error;
}

