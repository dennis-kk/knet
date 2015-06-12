# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (knet)

SET(CMAKE_C_FLAGS "-g -O2 -Wall")

add_executable(test_client
	test_client.c
)

add_executable(test_server
	test_server.c
)

target_link_libraries(test_client libknet.a -lpthread)
target_link_libraries(test_server libknet.a -lpthread)