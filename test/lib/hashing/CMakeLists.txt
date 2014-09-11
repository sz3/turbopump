cmake_minimum_required(VERSION 2.6)

project(hashing_test)

set (SOURCES
	test.cpp
	ConsistentHashRingTest.cpp
	HashTest.cpp
	LocateKeysTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/hashing
)

add_executable (
	hashing_test
	${SOURCES}
)

add_test(hashing_test hashing_test)

target_link_libraries(hashing_test
	hashing

	membership
	mock

	serialize
)

