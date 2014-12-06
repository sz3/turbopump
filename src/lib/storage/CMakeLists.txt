cmake_minimum_required(VERSION 2.6)

set(SOURCES
	readstream.cpp
	readstream.h
	writestream.cpp
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
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(storage STATIC ${SOURCES})


