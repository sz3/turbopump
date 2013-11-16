cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IPeerTracker.h
	PeerConnection.cpp
	PeerConnection.h
	PeerTracker.cpp
	PeerTracker.h
	ThrottledWriteStream.cpp
	ThrottledWriteStream.h
	WanPacketHandler.cpp
	WanPacketHandler.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(wan_server STATIC ${SOURCES})

