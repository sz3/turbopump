cmake_minimum_required(VERSION 2.6)

set(SOURCES
	ComputedOptions.cpp
	ComputedOptions.h

	ChainWrite.h
	IWatches.h
	MirrorToPeer.cpp
	MirrorToPeer.h
	ModifyPeer.cpp
	ModifyPeer.h
	NotifyWriteComplete.cpp
	NotifyWriteComplete.h
	RandomizedMirrorToPeer.cpp
	RandomizedMirrorToPeer.h
	SynchronizedWatches.cpp
	SynchronizedWatches.h
	Watches.cpp
	Watches.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(callbacks STATIC ${SOURCES})


