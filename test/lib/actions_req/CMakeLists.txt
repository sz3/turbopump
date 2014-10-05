cmake_minimum_required(VERSION 2.6)

project(actions_req_test)

set (SOURCES
	test.cpp
	MessageSenderTest.cpp
	WriteSupervisorTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/actions_req
)

add_executable (
	actions_req_test
	${SOURCES}
)

add_test(actions_req_test actions_req_test)

target_link_libraries(actions_req_test
-Wl,--start-group
	actions_req

	api
	common
	membership
	mock
	wan_server

	serialize
	socket
-Wl,--end-group

	msgpack
	tbb
)

