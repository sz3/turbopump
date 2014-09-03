cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IMessageSender.h
	MessageSender.cpp
	MessageSender.h
	ISuperviseWrites.h
	WriteSupervisor.cpp
	WriteSupervisor.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(actions_req STATIC ${SOURCES})


