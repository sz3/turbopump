#include "unittest.h"

#include "MerkleAction.h"
#include "cohesion/MerklePoint.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockSynchronize.h"
using std::string;

TEST_CASE( "MerkleActionTest/testOne", "default" )
{
	Peer peer("fooid");
	MockSynchronize sync;
	MerkleAction action(peer, sync);

	MerklePoint point;
	point.location.key = 1337;
	point.location.keybits = 32;
	point.hash = 8000;

	string data = MerklePointSerializer::toString(point);
	assertTrue( action.run(DataBuffer(data.data(), data.size())) );
	assertEquals( "compare(fooid none,1337 32 8000)", sync._history.calls() );
}

TEST_CASE( "MerkleActionTest/testMany", "default" )
{
	Peer peer("fooid");
	MockSynchronize sync;
	MerkleAction action(peer, sync);

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

	assertTrue( action.run(DataBuffer(data.data(), data.size())) );
	assertEquals( "compare(fooid none,1 1 10)|"
				  "compare(fooid none,2 2 20)|"
				  "compare(fooid none,3 3 30)", sync._history.calls() );
}


