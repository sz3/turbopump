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
	api
	common
	data_store
	deskew
	hashing
	logging
	main
	membership
	peer_client
	peer_server
	programmable
	user_server

	event
	file
	http
	serialize
	socket
	udt_socket
-Wl,--end-group

	msgpack
	pthread
	tbb
	udt
)

