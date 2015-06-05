cmake_minimum_required(VERSION 2.6)

set(SOURCES
	App.h
	App.cpp
	Interface.h
	Turbopump.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(turbopump STATIC ${SOURCES})
add_library(turbopump_shared SHARED ${SOURCES})

target_link_libraries(turbopump
-Wl,--start-group
	api
	callbacks
	common
	deskew
	hashing
	logging
	membership
	peer_client
	peer_server
	storage

	file
	serialize
	socket
	udt_socket
-Wl,--end-group

	msgpack
	tbb
	udt
)
target_link_libraries(turbopump_shared turbopump)

# rename turbopump_shared.so to turbopump.so
set_target_properties(turbopump_shared PROPERTIES OUTPUT_NAME turbopump)

install(
	TARGETS turbopump turbopump_shared
	DESTINATION ${turboproj_SOURCE_DIR}/dist/lib
)


