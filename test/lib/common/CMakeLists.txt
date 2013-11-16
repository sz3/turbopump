cmake_minimum_required(VERSION 2.6)

project(common_test)

set (SOURCES
	test.cpp
	ActionParserTest.cpp
	DataBufferTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/common
)

add_executable (
	common_test
	${SOURCES}
)

add_test(common_test common_test)

target_link_libraries(common_test

	common
)

