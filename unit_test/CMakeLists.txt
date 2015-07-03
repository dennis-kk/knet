# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (knet)

SET(CMAKE_CXX_FLAGS "-g -O2 -Wall -std=c++0x")

add_executable(unit_test
	testing.cpp
	unit_test.cpp
)

target_link_libraries(unit_test libknet.a -lpthread)
