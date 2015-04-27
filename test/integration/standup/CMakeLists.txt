cmake_minimum_required(VERSION 2.6)

project(standup_test)

set (SOURCES
	test.cpp
	StartupTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	standup_test
	${SOURCES}
)

add_test(standup_test standup_test)

target_link_libraries(standup_test
-Wl,--start-group
	command_line
	integration
	main
	turbopump
	user_server
-Wl,--end-group

	format
	http

	boost_filesystem
	boost_system
)

