/**
 * RPC单件类
 */
class {{@name}}_t {
public:
    /**
     * 析构
     */
    ~{{@name}}_t();

    /**
     * 取得单件指针
     * \return {{@name}}_t指针
     */
    static {{@name}}_t* instance();

    /**
     * 销毁单件
     */
    static void finalize();

    /**
     * 从stream_t读取RPC调用请求
     * \param stream stream_t实例
     * \retval error_ok 成功处理一次RPC调用
     * \retval error_rpc_not_enough_bytes 没有完整的RPC可以处理
     * \retval error_rpc_unmarshal_fail 处理RPC包字节流时读取失败
     * \retval error_rpc_unknown_id 读取到RPC调用，但RPC ID未注册
     * \retval error_rpc_cb_fail 调用RPC处理函数时，处理函数内部发生错误
     * \retval error_rpc_cb_fail_close 调用RPC处理函数时，处理函数内部发生错误，处理函数要求关闭stream_t相关联的管道
     * \retval error_rpc_cb_close 调用RPC处理函数后，处理函数要求关闭stream_t相关联的管道
     * \retval error_rpc_unknown_type RPC类型错误
     */
    int rpc_proc(stream_t* stream);

