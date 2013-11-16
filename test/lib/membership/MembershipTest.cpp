#include "unittest.h"

#include "Membership.h"

#include "Peer.h"
#include "file/FileRemover.h"
#include <string>

const std::string _myfile = "membership.backup";

TEST_CASE( "MembershipTest/testAdd", "default" )
{
	Membership membership(_myfile, "localhost:1337");
	membership.add("fooid");
	membership.add("barid");

	assertEquals( "barid none\nfooid none", membership.toString() );

	membership.addIp("1.2.3.4:80", "fooid");
	membership.addIp("8.7.6.5:1", "barid");
	membership.addIp("127.0.1.1:100", "barid");
	membership.addIp("derp", "derp");

	assertEquals( "barid 8.7.6.5:1|127.0.1.1:100\n"
				  "fooid 1.2.3.4:80", membership.toString() );
}

TEST_CASE( "MembershipTest/testLookup", "default" )
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

TEST_CASE( "MembershipTest/testSaveLoad", "default" )
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

