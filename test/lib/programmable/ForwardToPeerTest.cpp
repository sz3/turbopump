#include "unittest.h"

#include "ForwardToPeer.h"

#include "data_store/LocalDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "mock/MockIpSocket.h"
#include "mock/MockMembership.h"
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

TEST_CASE( "ForwardToPeerTest/testDefault", "[unit]" )
{
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	MockPeerTracker peers;
	ForwardToPeer command(membership, peers);

	// input
	TestableLocalDataStore store;
	store._store["dummy"].reset( new string("contents") );
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	assertTrue( command.run("file", reader) );

	assertEquals( "addIp(1.2.3.4,dude)|randomPeer()|self()", membership._history.calls() );
	assertEquals( "track(dude)", peers._history.calls() );
	assertEquals( "send(write|name=file|)|send(contents)|send()", mockSock->_history.calls() );
}

