cmake_minimum_required(VERSION 2.6)

project(load_test)

set (SOURCES
	test.cpp
	ReadWriteLoadTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
)

add_executable (
	load_test
	${SOURCES}
)

add_test(load_test load_test)

target_link_libraries(load_test

	membership

	command_line
	file
	serialize
	time
)

