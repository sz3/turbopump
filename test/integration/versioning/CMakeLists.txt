cmake_minimum_required(VERSION 2.6)

project(integration_versioning_test)

set (SOURCES
	test.cpp
	VersionChainingTest.cpp
	VersionSyncTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	integration_versioning_test
	${SOURCES}
)

add_test(integration_versioning_test integration_versioning_test)

target_link_libraries(integration_versioning_test
-Wl,--start-group
	common
	hashing
	membership
	integration

	command_line
	file
	format
	http
	serialize
-Wl,--end-group

	boost_filesystem
	boost_system
)

