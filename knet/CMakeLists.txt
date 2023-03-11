# CMakeLists file
cmake_minimum_required(VERSION 3.5)

project (knet)

if (MSVC)
elseif ()
    SET(CMAKE_C_FLAGS "-g -O2 -Wall")
endif()

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
	timer.c
	logger.c
	hash.c
	loop_profile.c
	trie.c
	ip_filter.c
	rb_tree.c
)

if (MSVC)
    foreach(var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        )
        if(${var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
        endif()
    endforeach()
    SET_TARGET_PROPERTIES(knet PROPERTIES PREFIX "lib")
    SET_TARGET_PROPERTIES(knet PROPERTIES SUFFIX ".a")
else()
    target_link_libraries(knet -lpthread)
endif()

INSTALL(TARGETS knet DESTINATION lib)
