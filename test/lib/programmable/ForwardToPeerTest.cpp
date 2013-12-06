#include "unittest.h"

#include "ForwardToPeer.h"

#include "data_store/IDataStoreReader.h"
#include "mock/MockDataStore.h"
#include "mock/MockIpSocket.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"
#include "wan_server/BufferedConnectionWriter.h"

#include "util/CallHistory.h"
#include <string>

using std::string;

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
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockIpSocket* mockSock = new MockIpSocket();
	peers._writer.reset(new BufferedConnectionWriter(std::shared_ptr<IIpSocket>(mockSock)));

	assertTrue( command.run("file", reader) );

	assertEquals( "addIp(1.2.3.4,dude)|randomPeer()", membership._history.calls() );
	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "send({0}{17}{}write|name=file|{0}{9}{0}contents{0}{1}{0})", mockSock->_history.calls() );
}

