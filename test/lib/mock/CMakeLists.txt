cmake_minimum_required(VERSION 2.6)

set(SOURCES
	MockAction.cpp
	MockAction.h
	MockBufferedConnectionWriter.cpp
	MockBufferedConnectionWriter.h
	MockDataStore.cpp
	MockDataStore.h
	MockDigestIndexer.cpp
	MockDigestIndexer.h
	MockDigestTree.cpp
	MockDigestTree.h
	MockHashRing.cpp
	MockHashRing.h
	MockKeyTabulator.cpp
	MockKeyTabulator.h
	MockLocateKeys.cpp
	MockLocateKeys.h
	MockLogger.cpp
	MockLogger.h
	MockMembership.cpp
	MockMembership.h
	MockMessageSender.cpp
	MockMessageSender.h
	MockMirrorToPeer.cpp
	MockMirrorToPeer.h
	MockPeerTracker.cpp
	MockPeerTracker.h
	MockProcessState.cpp
	MockProcessState.h
	MockSchedulerThread.cpp
	MockSchedulerThread.h
	MockSkewCorrector.cpp
	MockSkewCorrector.h
	MockSynchronize.cpp
	MockSynchronize.h
	MockUserPacketHandler.cpp
	MockUserPacketHandler.h
	MockWriteActionSender.cpp
	MockWriteActionSender.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(mock STATIC ${SOURCES})

