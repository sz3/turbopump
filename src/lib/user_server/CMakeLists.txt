cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IUserPacketHandler.h
	UserCommandContext.cpp
	UserCommandContext.h
	UserPacketHandler.cpp
	UserPacketHandler.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(user_server STATIC ${SOURCES})

