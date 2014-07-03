/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Membership.h"

#include "Peer.h"
#include "common/turbopump_defaults.h"
#include "file/FileRemover.h"
#include "mock/MockDataStore.h"
#include "serialize/StringUtil.h"
#include <string>
#include <vector>

const std::string _myfile = "membership.backup";

TEST_CASE( "MembershipTest/testAdd", "[unit]" )
{
	Membership membership(_myfile, "localhost:1337");
	membership.add("fooid");
	membership.add("barid");

	assertEquals( "barid none\nfooid none", membership.toString() );

	membership.addIp("1.2.3.4:80", "fooid");
	membership.addIp("1.2.3.4:80", "fooid");
	membership.addIp("1.2.3.4:80", "fooid");
	membership.addIp("8.7.6.5:1", "barid");
	membership.addIp("127.0.1.1:100", "barid");
	membership.addIp("derp", "derp");

	assertEquals( "barid 8.7.6.5:1|127.0.1.1:100\n"
				  "fooid 1.2.3.4:80", membership.toString() );
}

TEST_CASE( "MembershipTest/testLookup", "[unit]" )
{
	Membership membership(_myfile, "localhost:1337");
	membership.add("fooid");
	membership.addIp("1.2.3.4:80", "fooid");
	membership.add("barid");
	membership.addIp("5.6.7.8", "barid");

	assertEquals( "fooid", membership.lookupIp("1.2.3.4:80")->uid );
	assertEquals( "barid", membership.lookupIp("5.6.7.8")->uid );

	// doesn't exist == empty shared_ptr
	assertEquals( (void*)NULL, membership.lookupIp("12.21.21.78").get() );
	assertFalse( membership.lookupIp("lemon") );

	membership.addIp("someip", "fooid");
	assertEquals( "fooid", membership.lookupIp("someip")->uid );
	assertEquals( "fooid", membership.lookupIp("1.2.3.4:80")->uid );
}

TEST_CASE( "MembershipTest/testSaveLoad", "[unit]" )
{
	FileRemover remover(_myfile);
	Membership membership(_myfile, "localhost:1337");

	membership.add("barid");
	membership.add("fooid");
	membership.addIp("1.2.3.4:80", "fooid");
	membership.addIp("someip", "fooid");

	assertTrue( membership.save() );

	std::string contents;
	assertTrue( File::load(_myfile, contents) );
	assertEquals( "barid none\n"
				  "fooid 1.2.3.4:80|someip\n", contents );

	Membership other(_myfile, "localhost:1337");
	assertTrue( other.load() );
	assertEquals( membership.toString(), other.toString() );
	assertEquals( "fooid", other.lookupIp("1.2.3.4:80")->uid );
	assertEquals( "fooid", other.lookupIp("someip")->uid );
}

TEST_CASE( "MembershipTest/testAddSelf", "[unit]" )
{
	Membership membership(_myfile, "localhost:1337");

	assertTrue( membership.addSelf() );
	assertEquals( "1337 localhost:1337", membership.toString() );
	assertEquals( "1337", membership.self()->uid );
}

// special case, since UDT doesn't currently allow a new outgoing connection to use a bound port
TEST_CASE( "MembershipTest/testLoadFilterSelf", "[unit]" )
{
	FileRemover remover(_myfile);
	std::string contents = "barid localhost:9001\n"
						   "fooid localhost:1337\n";
	assertTrue( File::save(_myfile, contents) );

	Membership membership(_myfile, "localhost:1337");
	assertTrue( membership.load() );
	assertEquals( "barid", membership.lookupIp("localhost:9001")->uid );
	assertEquals( "barid", membership.lookupIp("localhost")->uid );
	assertFalse( membership.lookupIp("localhost:1337") );
}

TEST_CASE( "MembershipTest/testContainSelf", "[unit]" )
{
	// some slight gymnastics to load self...
	FileRemover remover(_myfile);
	std::string contents = "barid bar:9001\n"
						   "fooid foo:1337\n"
						   "myself localhost:1337\n";
	assertTrue( File::save(_myfile, contents) );

	Membership membership(_myfile, "localhost:1337");
	assertTrue( membership.load() );

	std::vector<std::string> list({"myself", "barid"});
	assertTrue( membership.containsSelf(list) );

	assertFalse( membership.containsSelf({"barid"}) );
	assertFalse( membership.containsSelf({"fooid", "barid"}) );
	assertFalse( membership.containsSelf({"oof", "rab"}) );
	assertTrue( membership.containsSelf({"fooid", "barid", "myself"}) );
	assertTrue( membership.containsSelf({"myself"}) );
}

TEST_CASE( "MembershipTest/testRandomPeerFromList", "[unit]" )
{
	Membership membership(_myfile, "localhost:1337");
	membership.add("fooid");
	membership.add("barid");

	std::vector<std::string> list;
	assertFalse( membership.randomPeerFromList(list) );

	list.push_back("fooid");
	std::shared_ptr<Peer> peer = membership.randomPeerFromList(list);
	assertEquals( "fooid", peer->uid );

	list[0] = "barid";
	peer = membership.randomPeerFromList(list);
	assertEquals( "barid", peer->uid );
}

TEST_CASE( "MembershipTest/testForEachPeer", "[unit]" )
{
	FileRemover remover(_myfile);
	Membership membership(_myfile, "localhost:1337");
	membership.add("fooid");
	membership.add("barid");
	membership.add("rabid");

	std::vector<std::string> peers;
	auto fun = [&peers] (const Peer& peer) { peers.push_back(peer.uid); };
	membership.forEachPeer(fun);

	assertStringsEqual("barid fooid rabid", StringUtil::join(peers));
}

TEST_CASE( "MembershipTest/testSyncToDataStore", "[unit]" )
{
	FileRemover remover(_myfile);
	Membership membership(_myfile, "localhost:1337");
	membership.add("fooid");
	membership.addIp("1.2.3.4", "fooid");
	membership.add("barid");
	membership.addIp("ip2", "barid");
	membership.add("rabid");
	membership.addIp("ip3", "rabid");

	MockDataStore store;
	membership.syncToDataStore(store);

	assertEquals( "Writer::write(.membership/barid,ip2)|Writer::commit(.membership/barid,0)"
				  "|Writer::write(.membership/fooid,1.2.3.4)|Writer::commit(.membership/fooid,0)"
				  "|Writer::write(.membership/rabid,ip3)|Writer::commit(.membership/rabid,0)", store._history.calls() );
	assertEquals( "1.2.3.4", store._store[".membership/fooid"] );
}

