cmake_minimum_required(VERSION 2.6)

set(SOURCES
	DataEntry.h
	IDataStore.h
	IDataStoreReader.h
	IDataStoreWriter.h
	RamDataStore.cpp
	RamDataStore.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(data_store STATIC ${SOURCES})
