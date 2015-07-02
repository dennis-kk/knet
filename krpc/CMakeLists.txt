# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (krpc)

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/cpp/
  ${PROJECT_SOURCE_DIR}/
)

SET(CMAKE_CXX_FLAGS "-g -O2 -Wall -std=c++0x")

add_executable(krpc
	krpc.cpp
	krpc_lexer.cpp
	krpc_parser.cpp
	krpc_ostream.cpp
	cpp/krpc_cpp.cpp
)

target_link_libraries(krpc)
ADD_SUBDIRECTORY(examples)
