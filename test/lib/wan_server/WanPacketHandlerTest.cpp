#include "unittest.h"

#include "WanPacketHandler.h"

#include "membership/Peer.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockIpSocket.h"
#include "mock/TestableDataStore.h"
#include "socket/IpAddress.h"
#include "socket/UdpSocket.h"
#include "wan_server/PeerConnection.h"

TEST_CASE( "WanPacketHandlerTest/testDefault", "default" )
{
	MockMembership membership;
	MockPeerTracker peers;
	TestableDataStore dataStore;
	WanPacketHandler handler(membership, peers, dataStore);

	UdpSocket sock(-1);
	sock.setTarget(IpAddress("1.2.3.4", 10));

	std::shared_ptr<IIpSocket> mockSock(new MockIpSocket());
	peers._conn.reset(new PeerConnection(mockSock));

	assertFalse( handler.onPacket(sock, "foo") );
	assertEquals( "", peers._history.calls() );

	membership._ips["1.2.3.4:10"].reset(new Peer("someguid"));
	assertFalse( handler.onPacket(sock, "foo") );
	assertEquals( "track(someguid)", peers._history.calls() );

	// first write, second write
	assertTrue( handler.onPacket(sock, "write|name=foo|i am a file") );
	assertTrue( handler.onPacket(sock, "write|name=bar|i am another file") );

	// second write flushes first write
	assertEquals( "i am a file", *dataStore._store["foo"] );
	assertEquals( NULL, dataStore._store["bar"].get() );
	assertEquals( "track(someguid)|track(someguid)|track(someguid)", peers._history.calls() );

	// more for second write + empty string to flush
	assertTrue( handler.onPacket(sock, " across two packets!") );
	assertTrue( handler.onPacket(sock, "") );
	assertEquals( "i am another file across two packets!", *dataStore._store["bar"] );
}
