/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ThreadLockedMerkleIndex.h"

#include "mock/MockMerkleIndex.h"
#include "mock/MockSchedulerThread.h"
using std::string;

TEST_CASE( "ThreadLockedMerkleIndexTest/testAdd", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.add("foo");
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "add(foo)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedMerkleIndexTest/testRemove", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.remove("foo");
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "remove(foo)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedMerkleIndexTest/testSplitTree", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.splitTree("foo");
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "splitTree(foo)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedMerkleIndexTest/testCannibalizeTree", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.cannibalizeTree("foo");
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "cannibalizeTree(foo)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedMerkleIndexTest/testFind", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.find("foo");
	assertEquals( "find(foo)", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}

TEST_CASE( "ThreadLockedMerkleIndexTest/testRandomTree", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.randomTree();
	assertEquals( "randomTree()", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}

TEST_CASE( "ThreadLockedMerkleIndexTest/testUnwantedTree", "[unit]" )
{
	MockMerkleIndex realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedMerkleIndex index(realIndex, scheduler);

	index.unwantedTree();
	assertEquals( "unwantedTree()", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}
