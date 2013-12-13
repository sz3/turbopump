#include "unittest.h"

#include "WanPacketHandler.h"

#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockSynchronize.h"
#include "programmable/Callbacks.h"
#include "programmable/TurboApi.h"
#include "wan_server/PeerConnection.h"

#include "event/SimpleExecutor.h"
#include "socket/IpAddress.h"
#include "socket/UdpSocket.h"
using std::string;

namespace {
	string formatPacket(unsigned char virtid, const string& packet)
	{
		string header{0, (char)(packet.size()+1), (char)virtid};
		return header + packet;
	}
}

TEST_CASE( "WanPacketHandlerTest/testDefault", "default" )
{
	SimpleExecutor executor;
	MockMembership membership;
	MockDataStore dataStore; // ahead of PeerTracker to avoid double free when test fails. :)
	MockPeerTracker peers;
	MockSynchronize sync;
	Callbacks callbacks;
	WanPacketHandler handler(executor, membership, peers, dataStore, sync, callbacks);

	UdpSocket sock(-1);
	sock.setTarget(IpAddress("1.2.3.4", 10));
	peers._conn.reset(new PeerConnection);

	assertFalse( handler.onPacket(sock, "foo") );
	assertEquals( "", peers._history.calls() );

	membership._ips["1.2.3.4:10"].reset(new Peer("someguid"));
	assertTrue( handler.onPacket(sock, "foo") );
	assertEquals( "track(someguid)", peers._history.calls() );

	peers._history.clear();

	// first write.
	// since we're using our own thread as the executor and we've already parsed a packet,
	// if we don't call end_processing beforehand, write will be queued but not processed.
	assertTrue( handler.onPacket(sock, formatPacket(33, "write|name=foo|i am a file")) );
	assertEquals( "", dataStore._store["foo"] );

	// second write. Clear the connection's processing flag so we do some work (processing both write|foo, and the first part of write|bar)
	peers._conn->end_processing();
	assertTrue( handler.onPacket(sock, formatPacket(35, "write|name=bar|i am another file")) );

	// second write begins and processes the first write
	assertEquals( "i am a file", dataStore._store["foo"] );
	assertEquals( "", dataStore._store["bar"] );
	assertEquals( "track(someguid)|track(someguid)", peers._history.calls() );

	// more for second write + empty string to signify EOF
	assertTrue( handler.onPacket(sock, formatPacket(35, " across two packets!")) );

	peers._conn->end_processing();
	assertTrue( handler.onPacket(sock, formatPacket(35, "")) );
	assertEquals( "i am another file across two packets!", dataStore._store["bar"] );
}

TEST_CASE( "WanPacketHandlerTest/testMultiplexing", "default" )
{
	SimpleExecutor executor;
	MockMembership membership;
	MockDataStore dataStore;
	MockPeerTracker peers;
	MockSynchronize sync;
	Callbacks callbacks;
	WanPacketHandler handler(executor, membership, peers, dataStore, sync, callbacks);

	UdpSocket sock(-1);
	sock.setTarget(IpAddress("1.2.3.4", 10));
	peers._conn.reset(new PeerConnection);

	membership._ips["1.2.3.4:10"].reset(new Peer("someguid"));
	string packet = formatPacket(35, "write|name=foo|i am a file") + formatPacket(37, "write|name=bar|another file") + formatPacket(37, "");
	assertTrue( handler.onPacket(sock, packet) );
	assertEquals( "track(someguid)", peers._history.calls() );

	// work through both writes
	assertEquals( "i am a file", dataStore._store["foo"] );
	assertEquals( "another file", dataStore._store["bar"] );
	assertEquals( "track(someguid)", peers._history.calls() );
}
