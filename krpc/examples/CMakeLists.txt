# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (rpc_sample)

SET(CMAKE_CXX_FLAGS "-g -O2 -Wall -std=c++0x")

add_executable(rpc_sample
	rpc.cpp
	rpc_sample.cpp
)

target_link_libraries(rpc_sample libknet.a -lpthread)
