cmake_minimum_required(VERSION 2.6)

project(callbacks_test)

set (SOURCES
	test.cpp
	AddPeerTest.cpp
	ChainWriteTest.cpp
	MirrorToPeerTest.cpp
	NotifyWriteCompleteTest.cpp
	RandomizedMirrorToPeerTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/callbacks
)

add_executable (
	callbacks_test
	${SOURCES}
)

add_test(callbacks_test callbacks_test)

target_link_libraries(callbacks_test
-Wl,--start-group
	callbacks

	common
	hashing
	mock
	membership
	peer_client
	peer_server
	storage

	serialize
	socket
-Wl,--end-group
)

