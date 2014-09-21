cmake_minimum_required(VERSION 2.6)

set(SOURCES
	Api.cpp
	Api.h
	ListKeys.h
	ListKeysOp.cpp
	ListKeysOp.h
	Op.h
	Request.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(api STATIC ${SOURCES})


