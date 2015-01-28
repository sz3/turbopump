/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "PartialTransfers.h"
#include "socket/MockSocketServer.h"
#include "socket/MockSocketWriter.h"
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
	MockSocketWriter one;
	one._endpoint = socket_address("1.2.3.4");
	transfers.add(one, helperOne.fun());
	assertEquals(1, transfers._transfers.size());
	assertEquals("waitForWriter(1.2.3.4:0)", server._history.calls());

	assertTrue( transfers.run(one) );
	assertEquals( "callback()", helperOne._history.calls() );
	assertEquals(0, transfers._transfers.size());
}

TEST_CASE( "PartialTransfersTest/testChain", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	TestHelper helperOne;
	MockSocketWriter one;
	one._endpoint = socket_address("1.2.3.4");
	transfers.add(one, helperOne.fun());
	assertEquals(1, transfers._transfers.size());

	TestHelper helperTwo;
	transfers.add(one, helperTwo.fun());
	assertEquals(1, transfers._transfers.size());
	assertEquals("waitForWriter(1.2.3.4:0)|waitForWriter(1.2.3.4:0)", server._history.calls());

	assertTrue( transfers.run(one) );
	assertEquals( "callback()", helperOne._history.calls() );
	assertEquals( "callback()", helperTwo._history.calls() );
	assertEquals(0, transfers._transfers.size());
}

TEST_CASE( "PartialTransfersTest/testRun.NothingToDo", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	MockSocketWriter one;
	one._endpoint = socket_address("1.2.3.4");
	assertTrue( transfers.run(one) );
}

TEST_CASE( "PartialTransfersTest/testRun.FunctionFails", "[unit]" )
{
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	MockSocketWriter one;
	one._endpoint = socket_address("1.2.3.4");
	transfers.add(one, [](){ return false; });

	TestHelper doesntRunAtFirst;
	transfers.add(one, doesntRunAtFirst.fun());
	assertFalse( transfers.run(one) );

	assertEquals( "", doesntRunAtFirst._history.calls() );
	assertEquals( 1, transfers._transfers.size() );

	assertTrue( transfers.run(one) );
	assertEquals( "callback()", doesntRunAtFirst._history.calls() );
	assertEquals( 0, transfers._transfers.size() );
}

TEST_CASE( "PartialTransfersTest/testRun.ManyFunctionsFail", "[unit]" )
{
	// previous test case, to the nth
	MockSocketServer server;
	TestablePartialTransfers transfers(server);

	MockSocketWriter one;
	one._endpoint = socket_address("1.2.3.4");

	for (int i = 0; i < 100; ++i)
		transfers.add(one, [](){ return false; });

	TestHelper doesntRunAtFirst;
	transfers.add(one, doesntRunAtFirst.fun());
	for (int i = 0; i < 100; ++i)
		assertFalse( transfers.run(one) );

	assertEquals( "", doesntRunAtFirst._history.calls() );
	assertEquals( 1, transfers._transfers.size() );

	assertTrue( transfers.run(one) );
	assertEquals( "callback()", doesntRunAtFirst._history.calls() );
	assertEquals( 0, transfers._transfers.size() );
}

