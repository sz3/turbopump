#include "unittest.h"

#include "Peer.h"
#include <string>

TEST_CASE( "PeerTest/testSerialization", "default" )
{
	Peer one("one");
	one.ips.push_back("ip1");
	one.ips.push_back("ip2");
	assertEquals( "one ip1|ip2", one.toString() );

	Peer other("");
	assertTrue( other.fromString(one.toString()) );
	assertEquals( "one", other.uid );
	assertEquals( 2, other.ips.size() );
	assertEquals( "ip1", other.ips[0] );
	assertEquals( "ip2", other.ips[1] );
}

TEST_CASE( "PeerTest/testSerializationNoIps", "default" )
{
	Peer one("one");
	assertEquals( "one none", one.toString() );

	Peer other("");
	assertTrue( other.fromString(one.toString()) );
	assertEquals( "one", other.uid );
	assertEquals( 0, other.ips.size() );
}

