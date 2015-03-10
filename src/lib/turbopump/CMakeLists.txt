cmake_minimum_required(VERSION 2.6)

set(SOURCES
	App.h
	App.cpp
	Turbopump.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(turbopump STATIC ${SOURCES})

target_link_libraries(turbopump
-Wl,--start-group
	api
	common
	deskew
	hashing
	logging
	membership
	peer_client
	peer_server
	programmable
	storage

	event
	file
	serialize
	socket
	udt_socket
-Wl,--end-group

	msgpack
	tbb
	udt
)
