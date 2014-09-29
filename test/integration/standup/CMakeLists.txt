cmake_minimum_required(VERSION 2.6)

project(standup_test)

set (SOURCES
	test.cpp
	StartupTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	standup_test
	${SOURCES}
)

add_test(standup_test standup_test)

target_link_libraries(standup_test
-Wl,--start-group
	actions
	actions_req
	api
	common
	data_store
	deskew
	hashing
	integration
	logging
	main
	membership
	programmable
	user_server
	wan_server

	command_line
	event
	file
	http
	serialize
	socket
	udt_socket
-Wl,--end-group

	boost_filesystem
	boost_system
	msgpack
	pthread
	tbb
	udt
)

