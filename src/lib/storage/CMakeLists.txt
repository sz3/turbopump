cmake_minimum_required(VERSION 2.6)

set(SOURCES
	readstream.h
	writestream.h
	IStore.h
	IReader.h
	IWriter.h
	FileReader.cpp
	FileReader.h
	FileStore.cpp
	FileStore.h
	FileWriter.cpp
	FileWriter.h
	StringReader.h
	SimpleFileStore.cpp
	SimpleFileStore.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(storage STATIC ${SOURCES})

target_link_libraries(storage
	attr
	boost_filesystem
	boost_system
)
