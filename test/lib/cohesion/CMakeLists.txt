cmake_minimum_required(VERSION 2.6)

project(cohesion_test)

set (SOURCES
	test.cpp
	MerkleIndexTest.cpp
	MerkleRangeTest.cpp
	MerkleTreeTest.cpp
	SkewCorrectorTest.cpp
	SynchronizerTest.cpp
	SynchronizerIntegrationTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/cohesion
)

add_executable (
	cohesion_test
	${SOURCES}
)

add_test(cohesion_test cohesion_test)

target_link_libraries(cohesion_test

	cohesion

	common
	consistent_hashing
	membership
	mock
	wan_server

	serialize
)

