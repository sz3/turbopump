cmake_minimum_required(VERSION 2.6)

set(SOURCES
	MockBufferedConnectionWriter.cpp
	MockBufferedConnectionWriter.h
	MockDataStore.cpp
	MockDataStore.h
	MockHashRing.cpp
	MockHashRing.h
	MockIpSocket.cpp
	MockIpSocket.h
	MockLogger.cpp
	MockLogger.h
	MockMembership.cpp
	MockMembership.h
	MockMerkleIndex.cpp
	MockMerkleIndex.h
	MockMerkleRing.cpp
	MockMerkleRing.h
	MockMerkleTree.cpp
	MockMerkleTree.h
	MockMessageSender.cpp
	MockMessageSender.h
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
	MockWriteActionSender.cpp
	MockWriteActionSender.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(mock STATIC ${SOURCES})

