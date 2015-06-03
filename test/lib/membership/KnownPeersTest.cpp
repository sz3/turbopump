/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "KnownPeers.h"

#include "Peer.h"
#include "common/MyMemberId.h"
#include "common/VectorClock.h"
#include "common/turbopump_defaults.h"
#include "file/FileRemover.h"
#include "mock/MockStore.h"
#include "serialize/str_join.h"
#include <memory>
#include <string>
#include <vector>
using std::shared_ptr;

namespace {
	const std::string _myfile = "peers.backup";

	class TestableKnownPeers : public KnownPeers
	{
	public:
		using KnownPeers::KnownPeers;
		using KnownPeers::addSelf;
	};
}

TEST_CASE( "KnownPeersTest/testUpdate", "[unit]" )
{
	KnownPeers peers(_myfile);
	assertTrue( peers.update("myself") );
	assertTrue( peers.update("fooid") );
	assertTrue( peers.update("barid") );

	assertEquals( "myself none\n"
				  "barid none\n"
				  "fooid none\n"
				  , peers.toString() );

	assertFalse( peers.update("myself", {"1.1.1.1:443"}) );
	assertFalse( peers.update("fooid", {"2.2.2.2:22"}) );
	assertFalse( peers.update("barid", {"3.3.3.3:1", "4.4.4.4:100"}) );

	assertEquals( "myself 1.1.1.1:443\n"
				  "barid 3.3.3.3:1|4.4.4.4:100\n"
				  "fooid 2.2.2.2:22\n"
				  , peers.toString() );
}


TEST_CASE( "KnownPeersTest/testLookup", "[unit]" )
{
	KnownPeers peers(_myfile);
	assertTrue( peers.update("3", {"127.0.0.1:9003"}) );
	assertTrue( peers.update("1", {"127.0.0.1:9001"}) );
	assertTrue( peers.update("2", {"127.0.0.1:9002"}) );

	{
		shared_ptr<Peer> peer = peers.lookup("1");
		assertFalse( !peer );
		assertEquals( "1", peer->uid );
	}
	{
		shared_ptr<Peer> peer = peers.lookup("2");
		assertFalse( !peer );
		assertEquals( "2", peer->uid );
	}

	// note that self() isn't in lookup...
	assertEquals( (void*)NULL, peers.lookup("3").get() );
	assertFalse( peers.lookup("3") );

	// but is instead in self()
	{
		shared_ptr<Peer> peer = peers.self();
		assertFalse( !peer );
		assertEquals( "3", peer->uid );
	}
}

TEST_CASE( "KnownPeersTest/testLookupAddr", "[unit]" )
{
	KnownPeers peers(_myfile);
	peers.update("me", {"127.0.0.1:1592"});
	peers.update("fooid", {"1.2.3.4:1592"});
	peers.update("barid", {"5.6.7.8:1592"});

	{
		shared_ptr<Peer> peer = peers.lookupAddr(socket_address("1.2.3.4", 1592));
		assertFalse( !peer );
		assertEquals( "fooid", peer->uid );
	}
	{
		shared_ptr<Peer> peer = peers.lookupAddr(socket_address("5.6.7.8"));
		assertFalse( !peer );
		assertEquals( "barid", peer->uid );
	}

	// doesn't exist == empty shared_ptr
	assertEquals( (void*)NULL, peers.lookupAddr(socket_address("8.8.8.8")).get() );
	assertFalse( peers.lookupAddr(socket_address()) );
}

TEST_CASE( "KnownPeersTest/testRemove", "[unit]" )
{
	KnownPeers peers(_myfile);
	assertTrue( peers.update("myself", {"localhost:1337"}) );
	assertTrue( peers.update("fooid", {"1.1.1.1:1592", "localhost:1592"}) );
	assertTrue( peers.update("barid", {"2.2.2.2:1592"}) );
	assertTrue( peers.update("noips") );

	assertEquals( "myself localhost:1337\n"
				  "barid 2.2.2.2:1592\n"
				  "fooid 1.1.1.1:1592|localhost:1592\n"
				  "noips none\n"
				  , peers.toString() );

	assertFalse( peers.remove("nobody") );

	assertTrue( !!peers.lookup("fooid") );
	assertEquals( "fooid", peers.lookupAddr(socket_address("localhost"))->uid );
	assertEquals( "fooid", peers.lookupAddr(socket_address("localhost", 1592))->uid );
	assertEquals( "fooid", peers.lookupAddr(socket_address("1.1.1.1"))->uid );
	assertEquals( "fooid", peers.lookupAddr(socket_address("1.1.1.1", 1592))->uid );

	assertTrue( peers.remove("fooid") );
	assertTrue( !peers.lookup("fooid") );
	assertTrue( !peers.lookupAddr(socket_address("localhost")) );
	assertTrue( !peers.lookupAddr(socket_address("localhost", 1592)) );
	assertTrue( !peers.lookupAddr(socket_address("1.1.1.1")) );
	assertTrue( !peers.lookupAddr(socket_address("1.1.1.1", 1592)) );

	assertEquals( "myself localhost:1337\n"
				  "barid 2.2.2.2:1592\n"
				  "noips none\n"
				  , peers.toString() );

	assertTrue( !!peers.lookup("barid") );
	assertEquals( "barid", peers.lookupAddr(socket_address("2.2.2.2"))->uid );
	assertEquals( "barid", peers.lookupAddr(socket_address("2.2.2.2", 1592))->uid );

	assertTrue( peers.remove("barid") );
	assertTrue( !peers.lookup("barid") );
	assertTrue( !peers.lookupAddr(socket_address("2.2.2.2")) );
	assertTrue( !peers.lookupAddr(socket_address("2.2.2.2", 1592)) );

	assertEquals( "myself localhost:1337\n"
				  "noips none\n"
				  , peers.toString() );

	assertTrue( !!peers.lookup("noips") );
	assertTrue( peers.remove("noips") );
	assertTrue( !peers.lookup("barid") );

	assertEquals( "myself localhost:1337\n"
				  , peers.toString() );
}

TEST_CASE( "KnownPeersTest/testAddSelf", "[unit]" )
{
	TestableKnownPeers peers(_myfile);

	assertTrue( peers.addSelf() );
	assertEquals( MyMemberId().str() + " none\n", peers.toString() );
	assertEquals( MyMemberId().str(), peers.self()->uid );
}

TEST_CASE( "KnownPeersTest/testSaveLoad", "[unit]" )
{
	FileRemover remover(_myfile);
	KnownPeers peers(_myfile);

	peers.update("me", {"localhost:1337"});
	peers.update("barid");
	peers.update("fooid", {"1.1.1.1:1592", "10.0.0.0:80"});

	assertTrue( peers.save() );

	std::string contents;
	assertTrue( File::load(_myfile, contents) );
	assertEquals( "me localhost:1337\n"
				  "fooid 1.1.1.1:1592|10.0.0.0:80\n"
				  "barid none\n"
				  , contents );

	KnownPeers other(_myfile);
	assertTrue( other.load() );
	assertEquals( peers.toString(), other.toString() );

	{
		shared_ptr<Peer> peer = other.lookupAddr(socket_address("1.1.1.1", 1592));
		assertFalse( !peer );
		assertEquals( "fooid", peer->uid );
	}
	{
		shared_ptr<Peer> peer = other.lookupAddr(socket_address("10.0.0.0", 80));
		assertFalse( !peer );
		assertEquals( "fooid", peer->uid );
	}
}

// special case, since UDT doesn't currently allow a new outgoing connection to use a bound port
TEST_CASE( "KnownPeersTest/testLoadFilterSelf", "[unit]" )
{
	FileRemover remover(_myfile);
	std::string contents = "fooid 200.200.200.200:22\n"
						   "barid 200.200.200.200:1592\n";
	assertTrue( File::save(_myfile, contents) );

	KnownPeers peers(_myfile);
	assertTrue( peers.load() );

	{
		shared_ptr<Peer> peer = peers.lookupAddr(socket_address("200.200.200.200", 1592));
		assertFalse( !peer );
		assertEquals( "barid", peer->uid );
	}
	{
		shared_ptr<Peer> peer = peers.lookupAddr(socket_address("200.200.200.200"));
		assertFalse( !peer );
		assertEquals( "barid", peer->uid );
	}

	// yes, even this one...
	{
		shared_ptr<Peer> peer = peers.lookupAddr(socket_address("200.200.200.200", 22));
		assertFalse( !peer );
		assertEquals( "barid", peer->uid );
	}
}

TEST_CASE( "KnownPeersTest/testForEachPeer", "[unit]" )
{
	FileRemover remover(_myfile);
	KnownPeers peers(_myfile);
	peers.update("me");
	peers.update("fooid");
	peers.update("barid");
	peers.update("rabid");

	std::vector<std::string> all;
	auto fun = [&all] (const Peer& peer) { all.push_back(peer.uid); };
	peers.forEachPeer(fun);

	assertStringsEqual("me fooid rabid barid", turbo::str::join(all));
}

TEST_CASE( "KnownPeersTest/testSyncToDataStore", "[unit]" )
{
	FileRemover remover(_myfile);
	KnownPeers peers(_myfile);
	peers.update("fooid", {"1.2.3.4"});

	MockStore store;
	store._writer = new MockStoreWriter();
	peers.syncToDataStore(store);

	assertEquals( "write(.membership/fooid," + VectorClock().increment("fooid").toString() + ",0)", store._history.calls() );
	assertEquals( "write(1.2.3.4)|flush()|close()|reader()", MockStoreWriter::calls() );
}

TEST_CASE( "KnownPeersTest/testSyncToDataStore.empty", "[unit]" )
{
	FileRemover remover(_myfile);
	KnownPeers peers(_myfile);
	peers.update("fooid", {"1.2.3.4"});

	MockStore store;
	peers.syncToDataStore(store);

	assertEquals( "write(.membership/fooid," + VectorClock().increment("fooid").toString() + ",0)", store._history.calls() );
	assertEquals( "", MockStoreWriter::calls() );
}

