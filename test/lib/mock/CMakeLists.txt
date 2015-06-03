cmake_minimum_required(VERSION 2.6)

set(SOURCES
	DummyTurbopumpApi.h
	MockBufferedConnectionWriter.cpp
	MockBufferedConnectionWriter.h
	MockCommand.h
	MockConsistentHashRing.cpp
	MockConsistentHashRing.h
	MockDigestIndexer.cpp
	MockDigestIndexer.h
	MockDigestTree.cpp
	MockDigestTree.h
	MockKeyTabulator.cpp
	MockKeyTabulator.h
	MockKnownPeers.cpp
	MockKnownPeers.h
	MockLocateKeys.cpp
	MockLocateKeys.h
	MockLogger.cpp
	MockLogger.h
	MockMessageSender.cpp
	MockMessageSender.h
	MockMirrorToPeer.cpp
	MockMirrorToPeer.h
	MockPartialTransfers.h
	MockPeerCommandCenter.cpp
	MockPeerCommandCenter.h
	MockProcessState.cpp
	MockProcessState.h
	MockRequestPacker.cpp
	MockRequestPacker.h
	MockScheduler.cpp
	MockScheduler.h
	MockSkewCorrector.cpp
	MockSkewCorrector.h
	MockStatusReporter.h
	MockStore.cpp
	MockStore.h
	MockStoreReader.cpp
	MockStoreReader.h
	MockStoreWriter.cpp
	MockStoreWriter.h
	MockSynchronize.cpp
	MockSynchronize.h
	MockUserPacketHandler.cpp
	MockUserPacketHandler.h
	MockWriteSupervisor.cpp
	MockWriteSupervisor.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(mock STATIC ${SOURCES})

