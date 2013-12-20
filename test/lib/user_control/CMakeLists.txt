cmake_minimum_required(VERSION 2.6)

project(user_control_test)

set (SOURCES
	test.cpp
	SwitchboardTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/user_control
)

add_executable (
	user_control_test
	${SOURCES}
)

add_test(user_control_test user_control_test)

target_link_libraries(user_control_test
-Wl,--start-group
	user_control

	actions
	common
	programmable
	membership
	mock

	serialize
	socket
-Wl,--end-group
)

