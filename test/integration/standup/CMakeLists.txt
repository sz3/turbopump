cmake_minimum_required(VERSION 2.6)

project(standup_test)

set (SOURCES
	test.cpp
	StartupTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/exe
)

add_executable (
	standup_test
	${SOURCES}
)

add_test(standup_test standup_test)

target_link_libraries(standup_test
-Wl,--start-group
	integration
	turbopump
	user_server
-Wl,--end-group

	http

	boost_filesystem
	boost_system
	pthread
)

