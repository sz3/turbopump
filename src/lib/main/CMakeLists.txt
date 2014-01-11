cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IProcessState.h
	ProcessState.cpp
	ProcessState.h
	TurboPumpApp.cpp
	TurboPumpApp.h
	WanServer.cpp
	WanServer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(main STATIC ${SOURCES})

