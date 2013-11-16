cmake_minimum_required(VERSION 2.6)

set(SOURCES
	ISwitchboard.h
	Switchboard.cpp
	Switchboard.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(user_control STATIC ${SOURCES})

