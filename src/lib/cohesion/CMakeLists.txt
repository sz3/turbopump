cmake_minimum_required(VERSION 2.6)

set(SOURCES
	DigestIndexer.cpp
	DigestIndexer.h
	DigestTree.cpp
	DigestTree.h
	ICorrectSkew.h
	IDigestIndexer.h
	IDigestKeys.h
	IKeyTabulator.h
	ISynchronize.h
	KeyRange.cpp
	KeyRange.h
	KeyTabulator.cpp
	KeyTabulator.h
	SkewCorrector.cpp
	SkewCorrector.h
	Synchronizer.cpp
	Synchronizer.h
	ThreadLockedKeyTabulator.cpp
	ThreadLockedKeyTabulator.h
	TreeId.h
	UniversalDigestIndexer.cpp
	UniversalDigestIndexer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(cohesion STATIC ${SOURCES})

