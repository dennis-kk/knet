# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (knet)

SET(CMAKE_CXX_FLAGS "-g -O2 -Wall -std=c++0x")

add_executable(unit_test
	testing.cpp
	unit_test.cpp
)

target_link_libraries(unit_test libknet.a -lpthread)
add_custom_command(TARGET unit_test
	POST_BUILD 
	COMMAND echo ${EXECUTABLE_OUTPUT_PATH}
	COMMAND cp -rf ${PROJECT_SOURCE_DIR}/ip_filter.ipf ${EXECUTABLE_OUTPUT_PATH}/
)
