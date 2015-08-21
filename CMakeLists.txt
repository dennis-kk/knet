# CMakeLists file
cmake_minimum_required(VERSION 2.6)

project (knet)

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/knet
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)

ADD_SUBDIRECTORY(knet)
ADD_SUBDIRECTORY(krpc)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(unit_test)


INSTALL(FILES
	${PROJECT_SOURCE_DIR}/include/address_api.h
	${PROJECT_SOURCE_DIR}/include/broadcast_api.h
	${PROJECT_SOURCE_DIR}/include/channel_ref_api.h
	${PROJECT_SOURCE_DIR}/include/config.h
	${PROJECT_SOURCE_DIR}/include/files.txt
	${PROJECT_SOURCE_DIR}/include/framework_api.h
	${PROJECT_SOURCE_DIR}/include/framework_config_api.h
	${PROJECT_SOURCE_DIR}/include/hash_api.h
	${PROJECT_SOURCE_DIR}/include/ip_filter_api.h
	${PROJECT_SOURCE_DIR}/include/knet.h
	${PROJECT_SOURCE_DIR}/include/logger_api.h
	${PROJECT_SOURCE_DIR}/include/loop_api.h
	${PROJECT_SOURCE_DIR}/include/loop_balancer_api.h
	${PROJECT_SOURCE_DIR}/include/loop_profile_api.h
	${PROJECT_SOURCE_DIR}/include/misc_api.h
	${PROJECT_SOURCE_DIR}/include/node_api.h
	${PROJECT_SOURCE_DIR}/include/node_config_api.h
	${PROJECT_SOURCE_DIR}/include/ringbuffer_api.h
	${PROJECT_SOURCE_DIR}/include/router_api.h
	${PROJECT_SOURCE_DIR}/include/rpc_api.h
	${PROJECT_SOURCE_DIR}/include/rpc_object_api.h
	${PROJECT_SOURCE_DIR}/include/stream_api.h
	${PROJECT_SOURCE_DIR}/include/thread_api.h
	${PROJECT_SOURCE_DIR}/include/timer_api.h
	${PROJECT_SOURCE_DIR}/include/trie_api.h
	${PROJECT_SOURCE_DIR}/include/version.h
	${PROJECT_SOURCE_DIR}/include/vrouter_api.h
DESTINATION include/knet)

ADD_TEST(unittest unit_test)
