cmake_minimum_required(VERSION 2.6)

project(user_server_test)

set (SOURCES
	test.cpp
	UserCommandContextTest.cpp
	UserPacketHandlerTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/user_server
)

add_executable (
	user_server_test
	${SOURCES}
)

add_test(user_server_test user_server_test)

target_link_libraries(user_server_test
-Wl,--start-group
	user_server

	api
	common
	hashing
	membership
	mock

	http
	serialize
	socket
-Wl,--end-group

	msgpackc
)

