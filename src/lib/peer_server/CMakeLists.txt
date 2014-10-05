cmake_minimum_required(VERSION 2.6)

set(SOURCES
	BufferedConnectionWriter.cpp
	BufferedConnectionWriter.h
	ConnectionWriteStream.cpp
	ConnectionWriteStream.h
	IBufferedConnectionWriter.h
	IPeerTracker.h
	OrderedPacket.h
	PacketParser.cpp
	PacketParser.h
	PeerConnection.cpp
	PeerConnection.h
	PeerTracker.cpp
	PeerTracker.h
	VirtualConnection.cpp
	VirtualConnection.h
	WanPacketHandler.cpp
	WanPacketHandler.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(peer_server STATIC ${SOURCES})

