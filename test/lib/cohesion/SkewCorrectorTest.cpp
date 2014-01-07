/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SkewCorrector.h"

#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockMerkleIndex.h"
#include "mock/MockWriteActionSender.h"

using std::string;

TEST_CASE( "SkewCorrectorTest/testPushKeyRange", "[unit]" )
{
	MockMerkleIndex index;
	MockDataStore store;
	MockWriteActionSender writer;
	SkewCorrector corrector(index, store, writer);

	index._enumerate.push_back("file1");
	index._enumerate.push_back("badfile");
	index._enumerate.push_back("file3");
	store._store["file1"] = "I am file 1";
	store._store["file3"] = "I am file 3";

	corrector.pushKeyRange(Peer("fooid"), 0, 1234567890);

	assertEquals( "enumerate(0,1234567890)", index._history.calls() );
	assertEquals( "store(fooid,file1)|store(fooid,file3)", writer._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Empty", "[unit]" )
{
	MockMerkleIndex index;
	MockDataStore store;
	MockWriteActionSender writer;
	SkewCorrector corrector(index, store, writer);

	corrector.pushKeyRange(Peer("fooid"), 0, 1234567890);

	assertEquals( "enumerate(0,1234567890)", index._history.calls() );
	assertEquals( "", writer._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.ConnectionExplodes", "[unit]" )
{
	MockMerkleIndex index;
	MockDataStore store;
	MockWriteActionSender writer;
	SkewCorrector corrector(index, store, writer);

	writer._storeFails = true;

	index._enumerate.push_back("file1");
	index._enumerate.push_back("badfile");
	index._enumerate.push_back("file3");
	store._store["file1"] = "I am file 1";
	store._store["file3"]= "I am file 3";

	corrector.pushKeyRange(Peer("fooid"), 0, 1234567890);

	assertEquals( "enumerate(0,1234567890)", index._history.calls() );
	assertEquals( "store(fooid,file1)", writer._history.calls() );
}
