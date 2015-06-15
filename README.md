# knet 1.3.x #
##

**knet** is cross-platform multi-threading light-weight TCP transport library, for now, knet support **IOCP**, **select**, **epoll** model on **Windows** and **CentOS**. it will testing under more Linux and Unix OS version, more model will be implemented like: kqueue(kevent) etc.   
**knet** is not a event loop like **libevent**, **knet** focus on transport only.

### loop ###

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

For more detail, see `examples/`

#### config.h ####

	#if defined(WIN32)
		#define LOOP_IOCP 1    /* IOCP */
		#define LOOP_SELECT 0  /* select */
	#else
		#define LOOP_EPOLL 1   /* epoll */
		#define LOOP_SELECT 0  /* select */
	#endif /* defined(WIN32) */

At the end of header file `knet/config.h`, change the macro value to tell compiler choose specific implement, IOCP on Windows, epoll on Linux by default, more implement will be appended in the future release.   

### balancer ###

`loop_t` runs at the thread which calling `loop_run` or `loop_run_once`, each `loop_t` knows nothing
about others. `loop_balancer_t` coordinates all attached `loop_t`(`loop_balance_attach`) and try to balance load for them.   

`loop_balancer_t` conceal the details and sophisticated for developer, the process of balancing is thread-safety  and highly efficent. **knet** offers a simple thread API to run internally `loop_run`, the interact among loops are also transparent for developers.

For more detail, see `examples/`

### build ###

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

- The command line of client   
 	1. `-ip`   
 		server listening IP
 	2. `-port`   
 		server listening port
    3. `-n`   
    	connector count   
- the command line of server   
	1. `-ip`   
 		server listening IP
 	2. `-port`   
 		server listening port   
    3. `-w`   
    	worker count
