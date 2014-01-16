cmake_minimum_required(VERSION 2.6)

set(SOURCES
	ICorrectSkew.h
	IMerkleIndex.h
	IMerkleTree.h
	ISynchronize.h
	MerkleIndex.cpp
	MerkleIndex.h
	MerkleRange.cpp
	MerkleRange.h
	MerkleTree.cpp
	MerkleTree.h
	SkewCorrector.cpp
	SkewCorrector.h
	Synchronizer.cpp
	Synchronizer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(cohesion STATIC ${SOURCES})

