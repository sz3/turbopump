cmake_minimum_required(VERSION 2.6)

set(SOURCES
	ICorrectSkew.h
	IMerkleIndex.h
	ISynchronize.h
	MerkleIndex.h
	MerkleIndex.cpp
	MerklePoint.h
	MerklePoint.cpp
	SkewCorrector.cpp
	SkewCorrector.h
	Synchronizer.cpp
	Synchronizer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(cohesion STATIC ${SOURCES})

