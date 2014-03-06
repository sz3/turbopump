/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ThreadLockedKeyTabulator.h"

#include "mock/MockKeyTabulator.h"
#include "mock/MockSchedulerThread.h"
using std::string;

TEST_CASE( "ThreadLockedKeyTabulatorTest/testAdd", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.update("foo", 0, 2);
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "update(foo,0,2)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedKeyTabulatorTest/testRemove", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.remove("foo", 2);
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "remove(foo,2)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedKeyTabulatorTest/testSplitSection", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.splitSection("foo");
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "splitSection(foo)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedKeyTabulatorTest/testCannibalizeSection", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.cannibalizeSection("foo");
	assertEquals( "schedule(0)", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "cannibalizeSection(foo)", realIndex._history.calls() );
}

TEST_CASE( "ThreadLockedKeyTabulatorTest/testFind", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.find("foo", 2);
	assertEquals( "find(foo,2)", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}

TEST_CASE( "ThreadLockedKeyTabulatorTest/testRandomTree", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.randomTree();
	assertEquals( "randomTree()", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}

TEST_CASE( "ThreadLockedKeyTabulatorTest/testUnwantedTree", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockSchedulerThread scheduler;
	ThreadLockedKeyTabulator index(realIndex, scheduler);

	index.unwantedTree();
	assertEquals( "unwantedTree()", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}
