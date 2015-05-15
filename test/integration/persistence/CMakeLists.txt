cmake_minimum_required(VERSION 2.6)

project(integration_persistence_test)

set (SOURCES
	test.cpp
	RestartTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	integration_persistence_test
	${SOURCES}
)

add_test(integration_persistence_test integration_persistence_test)

target_link_libraries(integration_persistence_test
-Wl,--start-group
	common
	hashing
	membership
	integration

	file
	http
	serialize
-Wl,--end-group

	boost_filesystem
	boost_system
)

