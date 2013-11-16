cmake_minimum_required(VERSION 2.6)

set(SOURCES
	circular_map.h
	IHashRing.h
	HashRing.cpp
	HashRing.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(consistent_hashing STATIC ${SOURCES})

target_link_libraries(consistent_hashing

	cryptopp
)
