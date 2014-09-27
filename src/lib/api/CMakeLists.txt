cmake_minimum_required(VERSION 2.6)

set(SOURCES
	Api.cpp
	Api.h
	Command.h
	Options.h
	Request.h

	Drop.h
	DropCommand.cpp
	DropCommand.h
	ListKeys.h
	ListKeysCommand.cpp
	ListKeysCommand.h
	Read.h
	ReadCommand.cpp
	ReadCommand.h
	Write.h
	WriteCommand.cpp
	WriteCommand.h
	WriteInstructions.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(api STATIC ${SOURCES})


