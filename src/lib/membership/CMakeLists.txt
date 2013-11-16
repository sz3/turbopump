cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IMembership.h
	Membership.cpp
	Membership.h
	Peer.cpp
	Peer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(membership STATIC ${SOURCES})

