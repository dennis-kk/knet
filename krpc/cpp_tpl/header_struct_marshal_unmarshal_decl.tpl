/**
 * {{@attribute_name}}序列化
 */
krpc_object_t* marshal({{@attribute_name}}& o);

/**
 * {{@attribute_name}}反序列化
 */
bool unmarshal(krpc_object_t* v, {{@attribute_name}}& o);

