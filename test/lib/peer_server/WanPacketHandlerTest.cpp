/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WanPacketHandler.h"

#include "membership/Peer.h"
#include "api/KeyRequest.h"
#include "api/Write.h"
#include "mock/DummyTurbopumpApi.h"
#include "mock/MockLogger.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"
#include "peer_client/MessagePacker.h"
#include "peer_server/PeerConnection.h"

#include "event/SimpleExecutor.h"
#include "socket/MockSocketWriter.h"
#include "socket/socket_address.h"
#include "msgpack.hpp"
using std::string;

namespace {
	MessagePacker _packer;

	string key_request(unsigned first, unsigned last)
	{
		Turbopump::KeyRequest req;
		req.first = first;
		req.last = last;

		msgpack::sbuffer sbuf;
		msgpack::pack(&sbuf, req);
		return _packer.package(req._ID, sbuf.data(), sbuf.size());
	}

	string write_request(string name, string data="")
	{
		Turbopump::Write req;
		req.name = name;

		msgpack::sbuffer sbuf;
		msgpack::pack(&sbuf, req);
		return _packer.package(req._ID, sbuf.data(), sbuf.size()) + data;
	}
}

TEST_CASE( "WanPacketHandlerTest/testProcessPendingBuffers", "default" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	MockMembership membership;
	MockLogger logger;
	MockPeerTracker peers;
	WanPacketHandler handler(api, executor, membership, peers, logger);

	PeerConnection conn;
	conn.pushRecv(_packer.package(33, "garbage bad action lulz"));
	conn.pushRecv(_packer.package(33, key_request(1, 10)));
	conn.pushRecv(_packer.package(33, key_request(2, 20)));
	conn.pushRecv(_packer.package(33, key_request(3, 30)));

	std::shared_ptr<Peer> peer(new Peer("someguid"));
	handler.processPendingBuffers(peer, conn);
	assertTrue( conn.empty() );
	assertEquals( "pushKeyRange(someguid,,3,1,10,)|pushKeyRange(someguid,,3,2,20,)|pushKeyRange(someguid,,3,3,30,)", api.corrector._history.calls() );
	assertTrue( !conn.command(33) );
}

TEST_CASE( "WanPacketHandlerTest/testProcessPendingBuffers_ConcurrentFileWrite", "default" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	MockMembership membership;
	MockLogger logger;
	MockPeerTracker peers;
	WanPacketHandler handler(api, executor, membership, peers, logger);

	PeerConnection conn;
	conn.pushRecv(_packer.package(33, write_request("foo", "i am a file")));
	conn.pushRecv(_packer.package(35, write_request("bar", "i am a different file")));
	conn.pushRecv(_packer.package(35, " with different bytes"));
	conn.pushRecv(_packer.package(33, " with more bytes!"));
	conn.pushRecv(_packer.package(35, ""));
	conn.pushRecv(_packer.package(33, ""));

	std::shared_ptr<Peer> peer(new Peer("someguid"));
	handler.processPendingBuffers(peer, conn);
	assertTrue( conn.empty() );
	assertEquals( "i am a file with more bytes!", api.dataStore._store["foo"] );
	assertEquals( "i am a different file with different bytes", api.dataStore._store["bar"] );
}

TEST_CASE( "WanPacketHandlerTest/testProcessPendingBuffers_ReuseOldVirtid", "default" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	MockMembership membership;
	MockLogger logger;
	MockPeerTracker peers;
	WanPacketHandler handler(api, executor, membership, peers, logger);

	PeerConnection conn;
	conn.pushRecv(_packer.package(33, write_request("foo", "i am a file")));
	conn.pushRecv(_packer.package(33, ""));
	conn.pushRecv(_packer.package(33, write_request("bar", "i am a different file")));
	conn.pushRecv(_packer.package(33, ""));

	std::shared_ptr<Peer> peer(new Peer("someguid"));
	handler.processPendingBuffers(peer, conn);
	assertTrue( conn.empty() );
	assertEquals( "i am a file", api.dataStore._store["foo"] );
	assertEquals( "i am a different file", api.dataStore._store["bar"] );
}

TEST_CASE( "WanPacketHandlerTest/testOnPacket", "default" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	MockMembership membership;
	MockLogger logger;
	MockPeerTracker peers;
	WanPacketHandler handler(api, executor, membership, peers, logger);

	MockSocketWriter sock;
	sock._endpoint = socket_address("1.2.3.4", 10);
	peers._conn.reset(new PeerConnection);

	string packet = _packer.package(32, "foo");
	assertFalse( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "", peers._history.calls() );

	membership._ips["1.2.3.4"].reset(new Peer("someguid"));
	assertTrue( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "track(someguid)", peers._history.calls() );

	peers._history.clear();

	// finally, send a properly formatted packet
	packet = _packer.package(33, key_request(1, 10));
	assertTrue( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "track(someguid)", peers._history.calls() );
	assertEquals( "pushKeyRange(someguid,,3,1,10,)", api.corrector._history.calls() );
}

TEST_CASE( "WanPacketHandlerTest/testOnPacketMultiplexing", "default" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	MockMembership membership;
	MockLogger logger;
	MockPeerTracker peers;
	WanPacketHandler handler(api, executor, membership, peers, logger);

	MockSocketWriter sock;
	sock._endpoint = socket_address("1.2.3.4", 10);
	peers._conn.reset(new PeerConnection);
	membership._ips["1.2.3.4"].reset(new Peer("someguid"));

	string packet = _packer.package(35, write_request("foo", "i am a file"))
			+ _packer.package(37, write_request("bar", "another file"))
			+ _packer.package(37, "") + _packer.package(35, "");
	assertTrue( handler.onPacket(sock, packet.data(), packet.size()) );
	assertEquals( "track(someguid)", peers._history.calls() );

	// work through both writes
	assertEquals( "i am a file", api.dataStore._store["foo"] );
	assertEquals( "another file", api.dataStore._store["bar"] );
	assertEquals( "track(someguid)", peers._history.calls() );
}


/*TEST_CASE( "WanPacketHandlerTest/testOnPacket_RecoverOnRetransmit", "default" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	MockMembership membership;
	MockLogger logger;
	MockPeerTracker peers;
	WanPacketHandler handler(executor, corrector, dataStore, ring, membership, messenger, peers, sync, logger, callbacks);

	MockSocketWriter sock;
	sock._target = IpAddress("1.2.3.4", 10);
	peers._conn.reset(new PeerConnection);
	membership._ips["1.2.3.4"].reset(new Peer("someguid"));

	// ostensibly, these are file contents for a file write we don't know about yet
	assertTrue( handler.onPacket(sock, _packer.package(32, "foo")) );
	assertTrue( handler.onPacket(sock, _packer.package(32, "")) ); // FIN
	assertTrue( handler.onPacket(sock, _packer.package(32, "bar")) );
	assertEquals( "", dataStore._store["foo"] );

	assertTrue( handler.onPacket(sock, _packer.package(32, "write|name=foo|see") + _packer.package(32, "smell")) );
	string actual = dataStore._store["foo"];
	assertEquals( "seesmellfoobar", actual );
}*/
