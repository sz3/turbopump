cmake_minimum_required(VERSION 2.6)

project(dynamic_membership_test)

set (SOURCES
	test.cpp
	DynamicMembershipTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
)

add_executable (
	dynamic_membership_test
	${SOURCES}
)

add_test(dynamic_membership_test dynamic_membership_test)

target_link_libraries(dynamic_membership_test
	common
	integration
	membership

	command_line
	file
	serialize
	time

	boost_filesystem
	boost_system
)

