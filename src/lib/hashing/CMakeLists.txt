cmake_minimum_required(VERSION 2.6)

set(SOURCES
	ConsistentHashRing.cpp
	ConsistentHashRing.h
	Hash.cpp
	Hash.h
	IConsistentHashRing.h
	ILocateKeys.h
	LocateKeys.cpp
	LocateKeys.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(hashing STATIC ${SOURCES})

target_link_libraries(hashing

	crypto_base
)
