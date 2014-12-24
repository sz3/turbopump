cmake_minimum_required(VERSION 2.6)

project(api_test)

set (SOURCES
	test.cpp
	AckWriteCommandTest.cpp
	AddPeerCommandTest.cpp
	ApiTest.cpp
	DeleteCommandTest.cpp
	DemandWriteCommandTest.cpp
	DropCommandTest.cpp
	HealKeyCommandTest.cpp
	KeyRequestCommandTest.cpp
	ListKeysTest.cpp
	ListKeysCommandTest.cpp
	OfferWriteCommandTest.cpp
	ReadCommandTest.cpp
	SyncCommandTest.cpp
	WriteTest.cpp
	WriteCommandTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
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
	hashing
	mock
	membership

	serialize
	socket

	msgpack
)

