cmake_minimum_required(VERSION 2.6)

set(SOURCES
	TurboPumpApp.cpp
	TurboPumpApp.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(main STATIC ${SOURCES})

