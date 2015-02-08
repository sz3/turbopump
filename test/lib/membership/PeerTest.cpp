/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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

TEST_CASE( "PeerTest/testActionId", "default" )
{
	Peer one("one");
	assertEquals( 1, one.nextActionId() );
	assertEquals( 2, one.nextActionId() );

	for (unsigned char i = 3; i != 0; ++i)
		assertEquals( i, one.nextActionId() );
	assertEquals( 1, one.nextActionId() );
}

