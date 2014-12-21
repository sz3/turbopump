/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ListKeysCommand.h"
#include "mock/MockStore.h"
#include "socket/StringByteStream.h"

TEST_CASE( "ListKeysCommandTest/testDefault", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);

	assertTrue( command.run() );
	assertEquals( "enumerate(1000)", store._history.calls() );
}

TEST_CASE( "ListKeysCommandTest/testNullWriter", "[unit]" )
{
	MockStore store;
	ListKeysCommand command(store);

	assertFalse( command.run() );
	assertEquals( "", store._history.calls() );
}

TEST_CASE( "ListKeysCommandTest/testPrint", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);

	command.print_key("one => 11|1,two:1");
	command.print_key("nuked => 11|2,deleted:1,two:1");
	command.print_key(".membership/peer => 11|1,two:1");

	assertEquals( "one => 11|1,two:1\n", stream.writeBuffer() );
}

TEST_CASE( "ListKeysCommandTest/testDeleted", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);
	command.params.deleted = true;

	command.print_key("one => 11|1,two:1");
	command.print_key("nuked => 11|2,deleted:1,two:1");
	command.print_key(".membership/peer => 11|1,two:1");

	assertEquals( "one => 11|1,two:1\n"
				  "nuked => 11|2,deleted:1,two:1\n", stream.writeBuffer() );
}

TEST_CASE( "ListKeysCommandTest/testAll", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);
	command.params.all = true;

	command.print_key("one => 11|1,two:1");
	command.print_key("nuked => 11|2,deleted:1,two:1");
	command.print_key(".membership/peer => 11|1,two:1");

	assertEquals( "one => 11|1,two:1\n"
				  ".membership/peer => 11|1,two:1\n", stream.writeBuffer() );
}
