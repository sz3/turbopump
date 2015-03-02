cmake_minimum_required(VERSION 2.6)

project(turbopumpd)

set (SOURCES
	main.cpp
)

add_executable (
	turbopumpd
	${SOURCES}
)

target_link_libraries(turbopumpd
-Wl,--start-group
	api
	common
	deskew
	hashing
	logging
	main
	membership
	peer_client
	peer_server
	programmable
	storage
	turbopump
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

