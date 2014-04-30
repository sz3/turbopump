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
	cohesion
	common
	consistent_hashing
	data_store
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
	time
	udt_socket
-Wl,--end-group

	boost_filesystem
	boost_system
	pthread
	tbb
	udt
)

