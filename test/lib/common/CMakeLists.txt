cmake_minimum_required(VERSION 2.6)

project(common_test)

set (SOURCES
	test.cpp
	DataBufferTest.cpp
	MerklePointTest.cpp
	MyMemberIdTest.cpp
	VectorClockTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/common
)

add_executable (
	common_test
	${SOURCES}
)

add_test(common_test common_test)

target_link_libraries(common_test
	common

	serialize
)

