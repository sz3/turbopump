cmake_minimum_required(VERSION 2.6)

project(load_test)

set (SOURCES
	test.cpp
	BigFileTest.cpp
	ReadWriteLoadTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	load_test
	${SOURCES}
)

add_test(load_test load_test)

target_link_libraries(load_test
-Wl,--start-group
	common
	hashing
	membership
	integration
	storage

	command_line
	file
	http
	serialize
	socket
-Wl,--end-group

	boost_filesystem
	boost_system
)

