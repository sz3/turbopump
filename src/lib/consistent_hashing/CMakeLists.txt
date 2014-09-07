cmake_minimum_required(VERSION 2.6)

set(SOURCES
	circular_map.h
	Hash.cpp
	Hash.h	
	HashRing.cpp
	HashRing.h
	IHashRing.h
	ILocateKeys.h
	LocateKeys.cpp
	LocateKeys.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(consistent_hashing STATIC ${SOURCES})

target_link_libraries(consistent_hashing

	crypto_base
)
