# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (knet)

SET(CMAKE_C_FLAGS "-g -O2 -Wall")
add_library(knet
	buffer.c
	channel.c
	channel_ref.c
	list.c
	loop.c
	loop_balancer.c
	loop_impl.c
	misc.c
	ringbuffer.c
	stream.c
	address.c
	version.c
	broadcast.c
	timer.c
	logger.c
	hash.c
	loop_profile.c
	trie.c
	ip_filter.c
	vrouter.c
	rb_tree.c
)

target_link_libraries(knet -lpthread)

INSTALL(TARGETS knet DESTINATION lib)
