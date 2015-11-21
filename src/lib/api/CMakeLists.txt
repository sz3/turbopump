cmake_minimum_required(VERSION 2.6)

set(SOURCES
	Api.cpp
	Api.h
	Command.h
	InternalCommand.h
	Options.h
	Request.h
	StreamingCommand.h

	AckWrite.h
	AckWriteCommand.cpp
	AckWriteCommand.h
	AddPeer.h
	AddPeerCommand.cpp
	AddPeerCommand.h
	Delete.h
	DeleteCommand.cpp
	DeleteCommand.h
	DemandWrite.h
	DemandWriteCommand.cpp
	DemandWriteCommand.h
	Drop.h
	DropCommand.cpp
	DropCommand.h
	KeyRequest.h
	KeyRequestCommand.cpp
	KeyRequestCommand.h
	ListKeys.h
	ListKeysCommand.cpp
	ListKeysCommand.h
	OfferWrite.h
	OfferWriteCommand.cpp
	OfferWriteCommand.h
	Read.h
	ReadCommand.cpp
	ReadCommand.h
	RemovePeer.h
	RemovePeerCommand.cpp
	RemovePeerCommand.h
	Status.h
	StatusCommand.cpp
	StatusCommand.h
	Sync.h
	SyncCommand.cpp
	SyncCommand.h
	Write.h
	WriteCommand.cpp
	WriteCommand.h
	WriteInstructions.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(api STATIC ${SOURCES})


