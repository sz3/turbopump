#include "unittest.h"

#include "WriteActionSender.h"

#include "data_store/LocalDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockIpSocket.h"
#include "mock/MockPeerTracker.h"
#include "wan_server/PeerConnection.h"
#include "util/CallHistory.h"
#include <string>

using std::string;

class TestableLocalDataStore : public LocalDataStore
{
public:
	using LocalDataStore::_store;
};

namespace {
	CallHistory _history;
}

TEST_CASE( "WriteActionSenderTest/testDefault", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers);

	// input
	TestableLocalDataStore store;
	store._store["dummy"].reset( new string("contents") );
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	assertTrue( client.store(Peer("dude"), "file", reader) );

	assertEquals( "track(dude)", peers._history.calls() );
	assertEquals( "send(write|name=file|)|send(contents)|send()", mockSock->_history.calls() );
}

