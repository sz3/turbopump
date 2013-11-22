cmake_minimum_required(VERSION 2.6)

project(consistent_hashing_test)

set (SOURCES
	test.cpp
	HashTest.cpp
	HashRingTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/consistent_hashing
)

add_executable (
	consistent_hashing_test
	${SOURCES}
)

add_test(consistent_hashing_test consistent_hashing_test)

target_link_libraries(consistent_hashing_test
	consistent_hashing
)

