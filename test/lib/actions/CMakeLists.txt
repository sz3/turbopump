cmake_minimum_required(VERSION 2.6)

project(actions_test)

set (SOURCES
	test.cpp
	AckWriteActionTest.cpp
	AddPeerActionTest.cpp
	DeleteActionTest.cpp
	DemandWriteActionTest.cpp
	DropActionTest.cpp
	HealKeyActionTest.cpp
	KeyReqActionTest.cpp
	LocalListActionTest.cpp
	LocalWriteActionTest.cpp
	OfferWriteActionTest.cpp
	ReadActionTest.cpp
	RemovePeerActionTest.cpp
	SyncActionTest.cpp
	WriteActionTest.cpp
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/actions
)

add_executable (
	actions_test
	${SOURCES}
)

add_test(actions_test actions_test)

target_link_libraries(actions_test

	actions
	common
	consistent_hashing
	deskew
	membership
	mock

	serialize
	socket
)

