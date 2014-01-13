cmake_minimum_required(VERSION 2.6)

set(SOURCES
	DropAction.cpp
	DropAction.h
	IAction.h
	KeyReqAction.cpp
	KeyReqAction.h
	LocalListAction.cpp
	LocalListAction.h
	LocalStateAction.cpp
	LocalStateAction.h
	MerkleAction.cpp
	MerkleAction.h
	MessageAction.cpp
	MessageAction.h
	ReadAction.cpp
	ReadAction.h
	ViewHashRingAction.cpp
	ViewHashRingAction.h
	ViewMembershipAction.cpp
	ViewMembershipAction.h
	WriteAction.cpp
	WriteAction.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(actions STATIC ${SOURCES})

