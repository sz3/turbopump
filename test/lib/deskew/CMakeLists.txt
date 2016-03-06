cmake_minimum_required(VERSION 2.6)

project(deskew_test)

set (SOURCES
	test.cpp
	DigestIndexerTest.cpp
	DigestTreeTest.cpp
	KeyRangeTest.cpp
	KeyTabulatorTest.cpp
	KeyTabulatorDynamicHashRingTest.cpp
	SkewCorrectorTest.cpp
	SynchronizerTest.cpp
	SynchronizerExchangeTest.cpp
	SynchronizedKeyTabulatorTest.cpp
	UniversalDigestIndexerTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/deskew
)

add_executable (
	deskew_test
	${SOURCES}
)

add_test(deskew_test deskew_test)

target_link_libraries(deskew_test
-Wl,--start-group
	deskew

	common
	hashing
	membership
	mock
	peer_server

	serialize
-Wl,--end-group
)

