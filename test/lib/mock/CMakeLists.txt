cmake_minimum_required(VERSION 2.6)

set(SOURCES
	MockIpSocket.cpp
	MockIpSocket.h
	MockMembership.cpp
	MockMembership.h
	MockMerkleIndex.cpp
	MockMerkleIndex.h
	MockMessageSender.cpp
	MockMessageSender.h
	MockPeerTracker.cpp
	MockPeerTracker.h
	MockSkewCorrector.cpp
	MockSkewCorrector.h
	MockSynchronize.cpp
	MockSynchronize.h
	TestableDataStore.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(mock STATIC ${SOURCES})

