cmake_minimum_required(VERSION 2.6)

project(programmable_test)

set (SOURCES
	test.cpp
	MirrorToPeerTest.cpp
	RandomizedMirrorToPeerTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/programmable
)

add_executable (
	programmable_test
	${SOURCES}
)

add_test(programmable_test programmable_test)

target_link_libraries(programmable_test
-Wl,--start-group
	programmable

	actions_req
	common
	data_store
	mock
	membership
	wan_server

	serialize
	socket
-Wl,--end-group

	tbb
)

