cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IProcessState.h
	IStatusReporter.h
	ProcessState.cpp
	ProcessState.h
	StatusReporter.h
	TurboPumpApp.cpp
	TurboPumpApp.h
	WanServer.cpp
	WanServer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(main STATIC ${SOURCES})

