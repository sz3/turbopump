cmake_minimum_required(VERSION 2.6)

project(integration_deletes_test)

set (SOURCES
	test.cpp
	KeyDeleteTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	integration_deletes_test
	${SOURCES}
)

add_test(integration_deletes_test integration_deletes_test)

target_link_libraries(integration_deletes_test
-Wl,--start-group
	common
	hashing
	membership
	integration

	command_line
	file
	http
	serialize
-Wl,--end-group

	boost_filesystem
	boost_system
)

