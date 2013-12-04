#include "unittest.h"

#include "WanPacketHandler.h"

#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockIpSocket.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockSynchronize.h"
#include "programmable/Callbacks.h"
#include "programmable/TurboApi.h"
#include "wan_server/PeerConnection.h"

#include "event/SimpleExecutor.h"
#include "socket/IpAddress.h"
#include "socket/UdpSocket.h"

TEST_CASE( "WanPacketHandlerTest/testDefault", "default" )
{
	SimpleExecutor executor;
	MockMembership membership;
	MockPeerTracker peers;
	MockDataStore dataStore;
	MockSynchronize sync;
	Callbacks callbacks;
	WanPacketHandler handler(executor, membership, peers, dataStore, sync, callbacks);

	UdpSocket sock(-1);
	sock.setTarget(IpAddress("1.2.3.4", 10));

	std::shared_ptr<IIpSocket> mockSock(new MockIpSocket());
	peers._conn.reset(new PeerConnection(mockSock));

	assertFalse( handler.onPacket(sock, "foo") );
	assertEquals( "", peers._history.calls() );

	membership._ips["1.2.3.4:10"].reset(new Peer("someguid"));
	assertTrue( handler.onPacket(sock, "foo") );
	assertEquals( "track(someguid)", peers._history.calls() );

	peers._history.clear();

	// first write.
	// since we're using our own thread as the executor,
	// if we don't call end_processing beforehand, write will be queued but not processed.
	assertTrue( handler.onPacket(sock, "write|name=foo|i am a file") );
	assertEquals( "", dataStore._store["foo"] );

	// second write. Clear the connection's processing flag so we do some work (processing both write|foo, and the first part of write|bar)
	peers._conn->end_processing();
	assertTrue( handler.onPacket(sock, "write|name=bar|i am another file") );

	// second write finishes and flushes first write
	assertEquals( "i am a file", dataStore._store["foo"] );
	assertEquals( "", dataStore._store["bar"] );
	assertEquals( "track(someguid)|track(someguid)", peers._history.calls() );

	// more for second write + empty string to signify EOF
	assertTrue( handler.onPacket(sock, " across two packets!") );

	peers._conn->end_processing();
	assertTrue( handler.onPacket(sock, "") );
	assertEquals( "i am another file across two packets!", dataStore._store["bar"] );
}
