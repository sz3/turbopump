cmake_minimum_required(VERSION 2.6)

project(programmable_test)

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
	${turboproj_SOURCE_DIR}/src/lib/programmable
)

add_executable (
	programmable_test
	${SOURCES}
)

add_test(programmable_test programmable_test)

target_link_libraries(programmable_test
-Wl,--start-group
	programmable

	common
	data_store
	hashing
	mock
	membership
	peer_client
	peer_server

	serialize
	socket
-Wl,--end-group

	tbb
)

