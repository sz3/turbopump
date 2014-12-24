cmake_minimum_required(VERSION 2.6)

project(membership_test)

set (SOURCES
	test.cpp
	MembershipTest.cpp
	PeerTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/membership
)

add_executable (
	membership_test
	${SOURCES}
)

add_test(membership_test membership_test)

target_link_libraries(membership_test
-Wl,--start-group
	membership
	common
	hashing
	mock

	file
	serialize
-Wl,--end-group
)

