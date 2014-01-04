cmake_minimum_required(VERSION 2.6)

set(SOURCES
	Callbacks.h
	Callbacks.cpp
	MirrorToPeer.cpp
	MirrorToPeer.h
	RandomizedMirrorToPeer.cpp
	RandomizedMirrorToPeer.h
	TurboApi.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(programmable STATIC ${SOURCES})


