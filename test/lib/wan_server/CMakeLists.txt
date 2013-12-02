cmake_minimum_required(VERSION 2.6)

project(wan_server_test)

set (SOURCES
	test.cpp
	WanPacketHandlerTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/wan_server
)

add_executable (
	wan_server_test
	${SOURCES}
)

add_test(wan_server_test wan_server_test)

target_link_libraries(wan_server_test
-Wl,--start-group
	wan_server

	actions
	common
	membership
	mock
	programmable

	serialize
	socket
-Wl,--end-group
)

