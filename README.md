# knet 1.4.x #
##

**knet** is cross-platform multi-threading light-weight TCP transport library, for now, knet support **IOCP**, **select**, **epoll** model on **Windows** and **CentOS**. it will testing(port) under more Linux and Unix OS version, more model will be implemented like: kqueue(kevent) etc.   
**knet** is not a event loop like **libevent**, **knet** focus on transport only.

**knet** 是跨平台，多线程，轻量级的TCP网络库，当前版本提供了Windows和Linux平台下的 **IOCP**, **select**, **epoll** 三种实现. 未来 **knet**将会在更多Linux和Unix操作系统下测试(移植),同时,更多的实现也会逐渐加入进来,譬如:kqueue(kevent)等.
**knet** 并不是像libevent一样的事件库, **knet** 只专注于传输.

### Loop ###
##

`kloop_t` is the wrapper of different implemented model, it's easy to use `kloop_t` to build a loop.    

`kloop_t` 是对不同平台实现的包装器，使用`kloop_t`可以非常容易的建立一个网络循环.

	#include "knet.h"
	kloop_t* loop = knet_loop_create();

For now, a loop is ready for run, we create a `channel_ref_t` as acceptor.   

网络循环已经建立好了，建立一个`channel_ref_t`作为监听器.

	kchannel_ref_t* channel = knet_loop_create_channel(loop, 0, 1024);
	kchannel_ref_accept(channel, "127.0.0.1", 80);

We built a acceptor channel with infinite send chain size and maximal read buffer size 1024, after that run endless loop.   

我们建立的监听器管道不限制发送队列的长度，读缓冲区的最大长度为1024, 建立完成后我们启动网络循环.

	knet_loop_run(loop);
	knet_loop_destroy(loop);

The program just holds your screen, we should add some code for new client coming.   

程序什么都没做，需要我们添加一些代码来建立客户端来的连接.

	void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
	    if (e & channel_cb_event_accept) {
	        /* TODO do job here */
	    }
	}

The whole story:   

整个过程如下:

	#include "knet.h"

	void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
	    if (e & channel_cb_event_accept) { /* the new client coming */
	        /* TODO do job here */
	    }
	}

	int main() {
		kloop_t* loop = knet_loop_create();
		kchannel_ref_t* channel = knet_loop_create_channel(loop, 0, 1024);
		knet_channel_ref_accept(channel, "127.0.0.1", 80);
		knet_channel_ref_set_cb(channel, acceptor_cb);
		knet_loop_run(loop);
		knet_loop_destroy(loop);
		return 0;
	}

For more detail, see `examples/basic.c`

#### config.h ####
##

	#if defined(WIN32)
		#define LOOP_IOCP 1    /* IOCP */
		#define LOOP_SELECT 0  /* select */
	#else
		#define LOOP_EPOLL 1   /* epoll */
		#define LOOP_SELECT 0  /* select */
	#endif /* defined(WIN32) */

In header file `knet/config.h`, change the macro value to tell compiler choose specific implement, IOCP on Windows, epoll on Linux by default, more implement will be appended in the future release.   

头文件`knet/config.h`内有一些有用的宏做条件编译，Windows的默认实现是IOCP, Linux的默认实现是epoll, 未来版本中会增加更多平台的实现.

	#define LOGGER_ON 0 /* the switch of internal logger */
	#define LOGGER_MODE (logger_mode_file | logger_mode_console | logger_mode_flush | logger_mode_override) /* the mode of logger */
	#define LOGGER_LEVEL logger_level_verbose /* the level of logger */

In header file `knet/logger.h`, `LOGGER_ON` is the switch of **knet** internal logger, the macro `LOGGER_MODE` and `LOGGER_LEVEL` can change the mode and the level of logger. Internal logger may help developer find the problom ASAP, `LOGGER_ON` should be set to 0 in release version.   

头文件`knet/logger.h`内，`LOGGER_ON`宏可以开启或关闭 **knet** 的内部日志，宏`LOGGER_MODE`和`LOGGER_LEVEL`分别表示日志模式和日志等级. 内部日志可以帮助使用者尽快的发现问题. `LOGGER_ON`宏在发行版本中应该被设置为零.

### Balancer ###
##

`kloop_t` runs at the thread which calling `knet_loop_run` or `knet_loop_run_once`, each `loop_t` knows nothing
about others. `kloop_balancer_t` coordinates all attached `kloop_t`(`knet_loop_balance_attach`) and try to balance load for them.   

`kloop_t`运行在调用`knet_loop_run`或者`knet_loop_run_once`函数的线程内,每个`loop_t`都不知道其他`loop_t`的存在. `kloop_balancer_t`协调所有与自己关联的`kloop_t`(`knet_loop_balance_attach`), 并对关联的网络循环做负载均衡.

`kloop_balancer_t` conceal the details and sophisticated for developer, the process of balancing is thread-safety  and highly efficent. **knet** offers a simple thread API to run internally `knet_loop_run_once`, the interact among loops are also transparent for developers.   

`kloop_balancer_t`为开发者隐藏了所有负载均衡细节, 负载均衡的过程是线程安全的, 同时也是非常高效的. **knet**提供了一个简单的线程API并在内部运行`knet_loop_run_once`, 网络循环之间的交互对于使用者也是透明的.

For more detail, see `examples/multi_loop.c`

### Framework ###
##

`framework_t` offers a general interface of multi-loop under multi-threading environment.

`framework_t`为多个网络循环在多线程的运行环境下提供了一套统一的接口.   

- framework starts a standalone acceptor/connector thread
- a thread pool to operate accepted/connected channels
- use the same callback type as the parameter of function `channel_ref_set_cb`    


- 框架启动一个独立的监听器/连接器线程   
- 使用线程池来负载管道的读/写操作   
- 使用与`channel_ref_set_cb`参数相同的管道回调函数   

The framework extremely simplifies code line for the startup and cleanup of loop under multi-threading environment.

框架非常大的简化了多线程环境下的启动与关闭流程所需的代码.   

	kframework_t* f = knet_framework_create();
	kframework_config_t* c = knet_framework_get_config(f);
	/* fork a new acceptor */
    kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, 0, 23);
    knet_framework_acceptor_config_set_client_cb(ac, client_cb);
    /* start framework, wait stop & destroy*/
    knet_framework_start_wait_destroy(f);
	
After framework started, you also can start additional acceptor or connector.

框架启动后，你也可以动态的添加更多的监听器或连接器.

	kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, 0, 23);
    knet_framework_acceptor_config_set_client_cb(ac, client_cb);
	knet_framework_acceptor_start(f, ac);

The callback function you set will be invoked in multiple threads(more than one worker), make sure the function thread-safety.

回调函数将在多线程环境下调用，确保你的回调函数是线程安全的.

For more detail, see `examples/framework.c`

### Node ###
##

`knode_t` API offers a high level(compare to `framework_t`) peer to peer node cluster abstract based on `kframework_t`. There two type of node: root node and node.  
Generally, the root node listening a port for node coming and notify other node in cluster the new joiner's identifier, after that new joiner connected with node cluster. Node only connect to which node type is concerned type(Use node configuration API). In fact, the root node could not be unique but it should be unique for a cluster. Node API are helpful to developer divide logical function to seperate process via node abstract, there are the feature list:   

`knode_t`接口在`kframework_t`功能基础上提供了更高层次的点对点的抽象. 一共有两种节点类型：根节点和普通节点.   
通常情况下，根节点监听一个端口等待其他节点接入同时将新接入的节点信息通知其他已接入的节点. 普通节点只接入自己关心的节点类型(通过API配置). 一个节点集群只能有一个跟节点（可以开启多个）, 节点API帮助使用者通过不同的进程和节点抽象并分割逻辑代码, 下面是功能列表:   


1. Node cluster
2. Peer-to-peer   
3. Broadcast message   
4. Broadcast message by node type   
5. Black IP filter   
6. White IP filter   
7. Internal heartbeat between node pair

1. 节点结群
2. 点对点
3. 广播
4. 广播到指定节点类型
5. IP黑名单
6. IP白名单
7. 节点间的心跳

For more detail, see the DEV document.

### RPC ###
##

**knet** offers a low level **RPC** object framework for developer. `krpc_t` is the base for the next version(1.5.x), developers may develop handwriting code by `krpc_t`, but its not a good way to maintain code for future development.

**knet**为使用者提供了一套低层次的 **RPC** 对象框架. `krpc_t`是下一个版本(1.5.x)的基础, 使用者可以使用`krpc_t`手写RPC通信,但这不是推荐方式，这样的代码非常难于维护.

For more detail, see `examples/rpc.c`

### RPC code generating - the next big version ###
##

**krpc** is a command line tool to generate RPC code for developes. The generated code utilizes RPC object framework to marshal/unmarshal the RPC method parameters, and a simple singleton RPC entry class.

`krpc -d output-directory -f input_rpc_file -n rpc_class_name -l the_language_to_generate`

After run the command, you'll find two files in the output-directory, `rpc_class_name.h` and `rpc_class_name.cpp`. For now, `krpc` supports C++ only.

For more detail, see

- `krpc/examples/rpc_sample.rpc`
- `krpc/examples/rpc_sample_object.rpc`
- `krpc/examples/rpc.cpp`
- `krpc/examples/rpc_sample.h`
- `krpc/examples/rpc_sample.cpp`

### Document ###
##

see `doc/knet.pptx` for big picture of **knet**   
see `doc/html/index.html` for dev doc.

### Build ###
##

1. Windows   
	see `knet/win-proj/knet-vs2010.sln`
2. Linux   
	./build.sh 

The build result in folder `knet/bin` and `knet/lib`.

### Test ###

see `knet/bin`.

1. Windows   
	`test_client-vs2010.exe`   
	`test_server-vs2010.exe`
2. Linux   
	`test_client`   
	`test_server`   
3. The command line of client
 	1. `-ip`   
 		server listening IP
 	2. `-port`   
 		server listening port
    3. `-n`   
    	connector count   
4. the command line of server   
	1. `-ip`   
 		server listening IP
 	2. `-port`   
 		server listening port   
    3. `-w`   
    	worker count
