cmake_minimum_required(VERSION 2.6)

set(SOURCES
	ICorrectSkew.h
	IMerkleIndex.h
	IMerkleRing.h
	IMerkleTree.h
	ISynchronize.h
	KeyRange.cpp
	KeyRange.h
	MerkleIndex.cpp
	MerkleIndex.h
	MerkleRing.cpp
	MerkleRing.h
	MerkleTree.cpp
	MerkleTree.h
	SkewCorrector.cpp
	SkewCorrector.h
	Synchronizer.cpp
	Synchronizer.h
	ThreadLockedMerkleIndex.cpp
	ThreadLockedMerkleIndex.h
	TreeId.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(cohesion STATIC ${SOURCES})

