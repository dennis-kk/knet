#include <iostream>
#include <sstream>
#include "rpc_sample.h"

int my_rpc_func(my_object_t& my_obj, std::vector<my_object_t>& obj_array, const std::string& test_str, std::vector<float>& array_f32) {
    std::stringstream ss;
    my_obj.print(ss);
    std::cout << ss.str();
    std::cout << "test_str=" << test_str << std::endl;
    return rpc_ok;
}

/* 客户端 - 连接器回调 */
void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_connect) { /* 连接成功 */
        /* 调用RPC函数 */
        my_object_t my_obj;
        my_obj.n_i8 = 0;
        my_obj.n_i16 = 1;
        my_obj.n_i32 = 2;
        my_obj.n_i64 = 3;
        my_obj.n_ui8 = 4;
        my_obj.n_ui16 = 5;
        my_obj.n_ui32 = 6;
        my_obj.n_ui64 = 7;
        my_message_t my_msg;
        my_msg.str = "hello world";
        my_obj.message.push_back(my_msg);
        std::vector<float> float_array;
        std::vector<my_object_t> obj_array;
        rpc_sample_t::instance()->my_rpc_func(stream, my_obj, obj_array, "hello world", float_array);
    }
}

/* 服务端 - 客户端回调 */
void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        if (error_ok == rpc_sample_t::instance()->rpc_proc(stream)) {
            /* 退出循环 */
            loop_exit(channel_ref_get_loop(channel));
        }
    }
}

/* 监听者回调 */
void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    if (e & channel_cb_event_accept) { /* 新连接 */
        /* 设置回调 */
        channel_ref_set_cb(channel, client_cb);
    }
}

int main() {
    /* 创建循环 */
    loop_t* loop = loop_create();
    /* 创建客户端 */
    channel_ref_t* connector = loop_create_channel(loop, 8, 1024);
    /* 创建监听者 */
    channel_ref_t* acceptor = loop_create_channel(loop, 8, 1024);
    /* 设置回调 */
    channel_ref_set_cb(connector, connector_cb);
    channel_ref_set_cb(acceptor, acceptor_cb);
    /* 监听 */
    channel_ref_accept(acceptor, 0, 80, 10);
    /* 连接 */
    channel_ref_connect(connector, "127.0.0.1", 80, 5);
    /* 启动 */
    loop_run(loop);
    /* 销毁, connector, acceptor不需要手动销毁 */
    loop_destroy(loop);
    return 0;
}
