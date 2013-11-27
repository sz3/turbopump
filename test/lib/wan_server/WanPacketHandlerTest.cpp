#include "unittest.h"

#include "WanPacketHandler.h"

#include "membership/Peer.h"
#include "mock/MockIpSocket.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockSynchronize.h"
#include "mock/TestableDataStore.h"
#include "programmable/Callbacks.h"
#include "programmable/TurboApi.h"
#include "socket/IpAddress.h"
#include "socket/UdpSocket.h"
#include "wan_server/PeerConnection.h"

TEST_CASE( "WanPacketHandlerTest/testDefault", "default" )
{
	MockMembership membership;
	MockPeerTracker peers;
	TestableDataStore dataStore;
	MockSynchronize sync;
	Callbacks callbacks;
	WanPacketHandler handler(membership, peers, dataStore, sync, callbacks);

	UdpSocket sock(-1);
	sock.setTarget(IpAddress("1.2.3.4", 10));

	std::shared_ptr<IIpSocket> mockSock(new MockIpSocket());
	peers._conn.reset(new PeerConnection(mockSock));

	assertFalse( handler.onPacket(sock, "foo") );
	assertEquals( "", peers._history.calls() );

	membership._ips["1.2.3.4:10"].reset(new Peer("someguid"));
	assertFalse( handler.onPacket(sock, "foo") );
	assertEquals( "decode(someguid,foo)", peers._history.calls() );

	peers._history.clear();

	// first write, second write
	assertTrue( handler.onPacket(sock, "write|name=foo|i am a file") );
	assertTrue( handler.onPacket(sock, "write|name=bar|i am another file") );

	// second write flushes first write
	assertEquals( "i am a file", *dataStore._store["foo"] );
	assertEquals( NULL, dataStore._store["bar"].get() );
	assertEquals( "decode(someguid,write|name=foo|i am a file)|"
				  "decode(someguid,write|name=bar|i am another file)", peers._history.calls() );

	// more for second write + empty string to flush
	assertTrue( handler.onPacket(sock, " across two packets!") );
	assertTrue( handler.onPacket(sock, "") );
	assertEquals( "i am another file across two packets!", *dataStore._store["bar"] );
}
