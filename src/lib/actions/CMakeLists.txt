cmake_minimum_required(VERSION 2.6)

set(SOURCES
	AddPeerAction.cpp
	AddPeerAction.h
	DeleteAction.cpp
	DeleteAction.h
	DemandWriteAction.cpp
	DemandWriteAction.h
	DropAction.cpp
	DropAction.h
	DropParams.h
	HealKeyAction.cpp
	HealKeyAction.h
	IAction.h
	KeyReqAction.cpp
	KeyReqAction.h
	LocalListAction.cpp
	LocalListAction.h
	LocalStateAction.cpp
	LocalStateAction.h
	LocalWriteAction.cpp
	LocalWriteAction.h
	MessageAction.cpp
	MessageAction.h
	OfferWriteAction.cpp
	OfferWriteAction.h
	ReadAction.cpp
	ReadAction.h
	RemovePeerAction.cpp
	RemovePeerAction.h
	SyncAction.cpp
	SyncAction.h
	ViewHashRingAction.cpp
	ViewHashRingAction.h
	ViewMembershipAction.cpp
	ViewMembershipAction.h
	WriteAction.cpp
	WriteAction.h
	WriteParams.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(actions STATIC ${SOURCES})

