# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (knet)

SET(CMAKE_C_FLAGS "-g -O2 -Wall")
add_executable(examples
	basic.c
	multi_connector.c
	connector_timeout.c
)
target_link_libraries(examples libknet.a -lpthread)