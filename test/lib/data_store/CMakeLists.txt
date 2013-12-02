cmake_minimum_required(VERSION 2.6)

project(data_store_test)

set (SOURCES
	test.cpp
	LocalDataStoreTest.cpp	
)

include_directories(
	${corelib_SOURCE_DIR}/test
	${dynamo_SOURCE_DIR}/test/lib
	${dynamo_SOURCE_DIR}/src/lib/data_store
)

add_executable (
	data_store_test
	${SOURCES}
)

add_test(data_store_test data_store_test)

target_link_libraries(data_store_test
	data_store

	tbb
)

