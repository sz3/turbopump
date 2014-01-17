cmake_minimum_required(VERSION 2.6)

set(SOURCES
	Callbacks.cpp
	Callbacks.h
	MirrorToPeer.cpp
	MirrorToPeer.h
	NotifyWriteComplete.cpp
	NotifyWriteComplete.h
	RandomizedMirrorToPeer.cpp
	RandomizedMirrorToPeer.h
	TurboApi.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(programmable STATIC ${SOURCES})


