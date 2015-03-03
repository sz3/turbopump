cmake_minimum_required(VERSION 2.6)

set(SOURCES
	App.h
	App.cpp
	Turbopump.h
	WanServer.cpp
	WanServer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(turbopump STATIC ${SOURCES})

