cmake_minimum_required(VERSION 2.6)

project(peer_client_test)

set (SOURCES
	test.cpp
	PartialTransfersTest.cpp
	MessagePackerTest.cpp
	MessageSenderTest.cpp
	WriteSupervisorTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/peer_client
)

add_executable (
	peer_client_test
	${SOURCES}
)

add_test(peer_client_test peer_client_test)

target_link_libraries(peer_client_test
-Wl,--start-group
	peer_client

	api
	common
	hashing
	membership
	mock
	peer_server

	serialize
	socket
-Wl,--end-group

	msgpackc
)

