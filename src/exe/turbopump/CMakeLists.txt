cmake_minimum_required(VERSION 2.6)

project(turbopump)

set (SOURCES
	main.cpp
)

add_executable (
	turbopump
	${SOURCES}
)

target_link_libraries(turbopump
-Wl,--start-group
	actions
	actions_req
	common
	consistent_hashing
	data_store
	deskew
	logging
	main
	membership
	programmable
	user_server
	wan_server

	event
	file
	http
	serialize
	socket
	udt_socket
-Wl,--end-group

	pthread
	tbb
	udt
)

