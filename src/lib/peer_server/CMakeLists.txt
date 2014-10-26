cmake_minimum_required(VERSION 2.6)

set(SOURCES
	BufferedConnectionWriter.cpp
	BufferedConnectionWriter.h
	ConcurrentCommandCenter.cpp
	ConcurrentCommandCenter.h
	ConnectionWriteStream.cpp
	ConnectionWriteStream.h
	IBufferedConnectionWriter.h
	IPeerCommandCenter.h
	MultiplexedSocketPool.h
	MultiplexedSocketWriter.cpp
	MultiplexedSocketWriter.h
	OrderedPacket.h
	PacketParser.cpp
	PacketParser.h
	PeerCommandRunner.cpp
	PeerCommandRunner.h
	PeerPacketHandler.cpp
	PeerPacketHandler.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(peer_server STATIC ${SOURCES})

