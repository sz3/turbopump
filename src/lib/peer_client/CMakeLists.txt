cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IMessagePacker.h
	IMessageSender.h
	ISuperviseWrites.h
	MessagePacker.cpp
	MessagePacker.h
	MessageSender.cpp
	MessageSender.h
	WriteSupervisor.cpp
	WriteSupervisor.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(peer_client STATIC ${SOURCES})


