cmake_minimum_required(VERSION 2.6)

set(SOURCES
	TurboCluster.cpp
	TurboCluster.h
	TurboRunner.cpp
	TurboRunner.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(integration STATIC ${SOURCES})

