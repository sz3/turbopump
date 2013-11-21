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
	cohesion
	common
	consistent_hashing
	data_store
	main
	membership
	programmable
	wan_server
	user_control

	event
	file
	serialize
	socket
-Wl,--end-group

	pthread
)

