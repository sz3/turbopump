/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "PartialTransfers.h"
#include "socket/MockSocketServer.h"
#include <functional>

namespace {
	class TestHelper
	{
	public:
		std::function<bool()> fun()
		{
			return std::bind(&TestHelper::callback, this);
		}

		bool callback()
		{
			_history.call("callback");
			return true;
		}

	public:
		CallHistory _history;
	};

	class TestablePartialTransfers : public PartialTransfers
	{
	public:
		using PartialTransfers::PartialTransfers;
		using PartialTransfers::_transfers;
	};
}

TEST_CASE( "PartialTransfersTest/testBasic", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	TestHelper helperOne;
	transfers.add(42, helperOne.fun());
	assertEquals(1, transfers._transfers.size());
	assertEquals("waitForWriter(42)", server._history.calls());

	assertTrue( transfers.run(42) );
	assertEquals( "callback()", helperOne._history.calls() );
	assertEquals(0, transfers._transfers.size());
}

TEST_CASE( "PartialTransfersTest/testChain", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	TestHelper helperOne;
	transfers.add(42, helperOne.fun());
	assertEquals(1, transfers._transfers.size());

	TestHelper helperTwo;
	transfers.add(42, helperTwo.fun());
	assertEquals(1, transfers._transfers.size());
	assertEquals("waitForWriter(42)|waitForWriter(42)", server._history.calls());

	assertTrue( transfers.run(42) );
	assertEquals( "callback()", helperOne._history.calls() );
	assertEquals( "callback()", helperTwo._history.calls() );
	assertEquals(0, transfers._transfers.size());
}

TEST_CASE( "PartialTransfersTest/testRun.NothingToDo", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	assertTrue( transfers.run(42) );
}

TEST_CASE( "PartialTransfersTest/testRun.FunctionFails", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	transfers.add(42, [](){ return false; });

	TestHelper doesntRunAtFirst;
	transfers.add(42, doesntRunAtFirst.fun());
	assertFalse( transfers.run(42) );

	assertEquals( "", doesntRunAtFirst._history.calls() );
	assertEquals( 1, transfers._transfers.size() );

	assertTrue( transfers.run(42) );
	assertEquals( "callback()", doesntRunAtFirst._history.calls() );
	assertEquals( 0, transfers._transfers.size() );
}

TEST_CASE( "PartialTransfersTest/testRun.ManyFunctionsFail", "[unit]" )
{
	// previous test case, to the nth
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	for (int i = 0; i < 100; ++i)
		transfers.add(42, [](){ return false; });

	TestHelper doesntRunAtFirst;
	transfers.add(42, doesntRunAtFirst.fun());
	for (int i = 0; i < 100; ++i)
		assertFalse( transfers.run(42) );

	assertEquals( "", doesntRunAtFirst._history.calls() );
	assertEquals( 1, transfers._transfers.size() );

	assertTrue( transfers.run(42) );
	assertEquals( "callback()", doesntRunAtFirst._history.calls() );
	assertEquals( 0, transfers._transfers.size() );
}

