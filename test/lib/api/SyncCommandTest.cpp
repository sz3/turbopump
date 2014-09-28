/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SyncCommand.h"
#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockSynchronize.h"
using std::string;

TEST_CASE( "SyncCommandTest/testOne", "[unit]" )
{
	MockSynchronize sync;
	SyncCommand command(sync);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.id = "oak";
	command.params.mirrors = 5;

	MerklePoint point;
	point.location.key = 1337;
	point.location.keybits = 32;
	point.hash = 8000;

	string data = MerklePointSerializer::toString(point);
	assertTrue( command.run(data.data(), data.size()) );
	assertEquals( "compare(peer,oak,5,1337 32 8000,0)", sync._history.calls() );
}

TEST_CASE( "SyncCommandTest/testMany", "[unit]" )
{
	MockSynchronize sync;
	SyncCommand command(sync);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.id = "oak";

	string data;
	for (int i = 1; i <= 3; ++i)
	{
		if (i != 1)
			data += "|";
		MerklePoint point;
		point.location.key = i;
		point.location.keybits = i;
		point.hash = i*10;
		data += MerklePointSerializer::toString(point);
	}

	assertTrue( command.run(data.data(), data.size()) );
	assertEquals( "compare(peer,oak,3,1 1 10,0)|"
				  "compare(peer,oak,3,2 2 20,0)|"
				  "compare(peer,oak,3,3 3 30,1)", sync._history.calls() );
}


