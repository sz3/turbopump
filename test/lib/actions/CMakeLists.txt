cmake_minimum_required(VERSION 2.6)

project(actions_test)

set (SOURCES
	test.cpp
	KeyReqActionTest.cpp
	MerkleActionTest.cpp
	WriteActionTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/actions
)

add_executable (
	actions_test
	${SOURCES}
)

add_test(actions_test actions_test)

target_link_libraries(actions_test

	actions
	cohesion
	common
	membership
	mock

	serialize
)

