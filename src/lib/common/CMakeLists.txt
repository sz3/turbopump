cmake_minimum_required(VERSION 2.6)

set(SOURCES
	turbopump_defaults.h
	ActionParser.cpp
	ActionParser.h
	DataBuffer.cpp
	DataBuffer.h
	KeyMetadata.h
	MerklePoint.cpp
	MerklePoint.h
	MyMemberId.cpp
	MyMemberId.h
	VectorClock.cpp
	VectorClock.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(common STATIC ${SOURCES})

