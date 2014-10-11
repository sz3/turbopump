cmake_minimum_required(VERSION 2.6)

project(peer_server_test)

set (SOURCES
	test.cpp
	BufferedConnectionWriterTest.cpp
	BufferedConnectionWriterThreadingTest.cpp
	PeerTrackerTest.cpp
	WanPacketHandlerTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/peer_server
)

add_executable (
	peer_server_test
	${SOURCES}
)

add_test(peer_server_test peer_server_test)

target_link_libraries(peer_server_test
-Wl,--start-group
	peer_server

	api
	common
	hashing
	membership
	mock
	peer_client
	programmable

	event
	serialize
	socket
-Wl,--end-group

	msgpack
	tbb
)

