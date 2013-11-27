cmake_minimum_required(VERSION 2.6)

project(cohesion_test)

set (SOURCES
	test.cpp
	MerkleIndexTest.cpp
	SkewCorrectorTest.cpp
	SynchronizerTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/cohesion
)

add_executable (
	cohesion_test
	${SOURCES}
)

add_test(cohesion_test cohesion_test)

target_link_libraries(cohesion_test
-Wl,--start-group
	cohesion

	common
	consistent_hashing
	data_store
	membership
	mock
	wan_server

	serialize
	socket
-Wl,--end-group
)

