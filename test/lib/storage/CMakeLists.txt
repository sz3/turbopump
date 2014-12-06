cmake_minimum_required(VERSION 2.6)

project(storage_test)

set (SOURCES
	test.cpp
	FileReaderTest.cpp
	FileStoreTest.cpp
	FileWriterTest.cpp
	readstreamTest.cpp
)

include_directories(
	${turbolib_SOURCE_DIR}/test
	${turboproj_SOURCE_DIR}/test/lib
	${turboproj_SOURCE_DIR}/src/lib/storage
)

add_executable (
	storage_test
	${SOURCES}
)

add_test(storage_test storage_test)

target_link_libraries(storage_test
	storage
	common
	hashing

	file
	serialize
	socket

	boost_filesystem
	boost_system
)
