cmake_minimum_required(VERSION 2.6)

project(integration_partition_test)

set (SOURCES
	test.cpp
	MerklePartitionTest.cpp
	PartitionStoreTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	integration_partition_test
	${SOURCES}
)

add_test(integration_partition_test integration_partition_test)

target_link_libraries(integration_partition_test
-Wl,--start-group
	common
	hashing
	membership
	integration

	command_line
	file
	http
	serialize
	time
-Wl,--end-group

	boost_filesystem
	boost_system
)

