/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ListKeysCommand.h"
#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "socket/StringByteStream.h"

TEST_CASE( "ListKeysCommandTest/testDefault", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	ListKeysCommand command(store, stream);

	assertTrue( command.run(DataBuffer::Null()) );
	assertEquals( "report(0,.membership/)", store._history.calls() );
}

TEST_CASE( "ListKeysCommandTest/testDeleted", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	ListKeysCommand command(store, stream);
	command.params.deleted = true;

	assertTrue( command.run(DataBuffer::Null()) );
	assertEquals( "report(1,.membership/)", store._history.calls() );
}

TEST_CASE( "ListKeysCommandTest/testAll", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	ListKeysCommand command(store, stream);
	command.params.all = true;

	assertTrue( command.run(DataBuffer::Null()) );
	assertEquals( "report(0,)", store._history.calls() );
}
