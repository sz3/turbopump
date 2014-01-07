cmake_minimum_required(VERSION 2.6)

project(partition_test)

set (SOURCES
	test.cpp
	PartitionStoreTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
)

add_executable (
	partition_test
	${SOURCES}
)

add_test(partition_test partition_test)

target_link_libraries(partition_test
-Wl,--start-group
	membership
	integration

	command_line
	file
	serialize
	time
-Wl,--end-group
)

