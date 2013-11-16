cmake_minimum_required(VERSION 2.6)

set(SOURCES
	IAction.h
	LocalListAction.cpp
	LocalListAction.h
	MerkleAction.cpp
	MerkleAction.h
	MessageAction.cpp
	MessageAction.h
	ReadAction.cpp
	ReadAction.h
	ViewMembershipAction.cpp
	ViewMembershipAction.h
	WriteAction.cpp
	WriteAction.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(actions STATIC ${SOURCES})

