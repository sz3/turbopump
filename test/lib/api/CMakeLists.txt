cmake_minimum_required(VERSION 2.6)

project(api_test)

set (SOURCES
	test.cpp
	ApiTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/api
)

add_executable (
	api_test
	${SOURCES}
)

add_test(api_test api_test)

target_link_libraries(api_test
	api

	common
	mock

	socket
	msgpack
)

