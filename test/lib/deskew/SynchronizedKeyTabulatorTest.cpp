/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SynchronizedKeyTabulator.h"

#include "mock/MockKeyTabulator.h"
#include "mock/MockScheduler.h"
using std::string;

TEST_CASE( "SynchronizedKeyTabulatorTest/testAdd", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.update("foo", 0, 2);
	assertEquals( "execute()", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "update(foo,0,2)", realIndex._history.calls() );
}

TEST_CASE( "SynchronizedKeyTabulatorTest/testRemove", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.remove("foo", 2);
	assertEquals( "execute()", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "remove(foo,2)", realIndex._history.calls() );
}

TEST_CASE( "SynchronizedKeyTabulatorTest/testSplitSection", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.splitSection("foo");
	assertEquals( "execute()", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "splitSection(foo)", realIndex._history.calls() );
}

TEST_CASE( "SynchronizedKeyTabulatorTest/testCannibalizeSection", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.cannibalizeSection("foo");
	assertEquals( "execute()", scheduler._history.calls() );

	scheduler.run();
	assertEquals( "cannibalizeSection(foo)", realIndex._history.calls() );
}

TEST_CASE( "SynchronizedKeyTabulatorTest/testFind", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.find("foo", 2);
	assertEquals( "find(foo,2)", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}

TEST_CASE( "SynchronizedKeyTabulatorTest/testRandomTree", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.randomTree();
	assertEquals( "randomTree()", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}

TEST_CASE( "SynchronizedKeyTabulatorTest/testUnwantedTree", "[unit]" )
{
	MockKeyTabulator realIndex;
	MockScheduler scheduler;
	SynchronizedKeyTabulator index(realIndex, scheduler);

	index.unwantedTree();
	assertEquals( "unwantedTree()", realIndex._history.calls() );
	assertEquals( "", scheduler._history.calls() );
}
