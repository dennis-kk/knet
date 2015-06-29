# knet 1.4.x #
##

**knet** is cross-platform multi-threading light-weight TCP transport library, for now, knet support **IOCP**, **select**, **epoll** model on **Windows** and **CentOS**. it will testing under more Linux and Unix OS version, more model will be implemented like: kqueue(kevent) etc.   
**knet** is not a event loop like **libevent**, **knet** focus on transport only.

### loop ###
##

`loop_t` is the wrapper of different implemented model, it's easy to use `loop_t` to build a loop.    

	#include "knet.h"
	loop_t* loop = loop_create();

For now, a loop is ready for run, we create a `channel_ref_t` as acceptor.

	channel_ref_t* channel = loop_create_channel(loop, 0, 1024);
	channel_ref_accept(channel, "127.0.0.1", 80);

We built a acceptor channel with infinite send chain size and maximal read buffer size 1024, after that run endless loop.

	loop_run(loop);
	loop_destroy(loop);

The program just holds your screen, we should add some code for new client coming.

	void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
	    if (e & channel_cb_event_accept) {
	        /* TODO do job here */
	    }
	}

The whole story:

	#include "knet.h"

	void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
	    if (e & channel_cb_event_accept) { /* the new client coming */
	        /* TODO do job here */
	    }
	}

	int main() {
		loop_t* loop = loop_create();
		channel_ref_t* channel = loop_create_channel(loop, 0, 1024);
		channel_ref_accept(channel, "127.0.0.1", 80);
		channel_ref_set_cb(channel, acceptor_cb);
		loop_run(loop);
		loop_destroy(loop);
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

	#define LOGGER_ON 0 /* the switch of internal logger */
	#define LOGGER_MODE (logger_mode_file | logger_mode_console | logger_mode_flush | logger_mode_override) /* the mode of logger */
	#define LOGGER_LEVEL logger_level_verbose /* the level of logger */

`LOGGER_ON` is the switch of **knet** internal logger, the macro `LOGGER_MODE` and `LOGGER_LEVEL` can change the mode and the level of logger. Internal logger may help developer find the problom ASAP, `LOGGER_ON` should be set to 0 in release version.

### balancer ###
##

`loop_t` runs at the thread which calling `loop_run` or `loop_run_once`, each `loop_t` knows nothing
about others. `loop_balancer_t` coordinates all attached `loop_t`(`loop_balance_attach`) and try to balance load for them.   

`loop_balancer_t` conceal the details and sophisticated for developer, the process of balancing is thread-safety  and highly efficent. **knet** offers a simple thread API to run internally `loop_run`, the interact among loops are also transparent for developers.

For more detail, see `examples/multi_loop.c`

### rpc ###
##

**knet** offers a low level **RPC** object framework for developer. `krpc_t` is the base for the next version(1.5.x), developers may develop handwriting code by `krpc_t`, but its not a good way to maintain code for future development.

For more detail, see `examples/rpc.c`

### rpc code generating ###
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

### build ###
##

1. Windows   
	see `knet/win-proj/knet-vs2010.sln`
2. Linux   
	see `knet/CMakeLists.txt`   
	`cmake CMakeLists.txt`   
	`make`   

The build result in folder `knet/bin`.

### test ###

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
