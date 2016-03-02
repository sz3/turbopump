/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ListKeysCommand.h"
#include "common/KeyMetadata.h"
#include "mock/MockStore.h"
#include "socket/StringByteStream.h"
using std::string;

TEST_CASE( "ListKeysCommandTest/testDefault", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);

	assertTrue( command.run() );
	assertEquals( "enumerate(1000,)", store._history.calls() );
}

TEST_CASE( "ListKeysCommandTest/testPrefix", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);
	command.params.prefix = "foo/bar";

	assertTrue( command.run() );
	assertEquals( "enumerate(1000,foo/bar)", store._history.calls() );
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

	command.print_key("one", KeyMetadata(), "11:1,two.1");
	command.print_key("nuked", KeyMetadata(), "11:2,delete.1,two.1");
	command.print_key(".membership/peer", KeyMetadata(), "11:1,two.1");

	string expected = R"("one":"11:1,two.1",
)";
	assertEquals( expected, stream.writeBuffer() );
}

TEST_CASE( "ListKeysCommandTest/testDeleted", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);
	command.params.deleted = true;

	command.print_key("one", KeyMetadata(), "11:1,two.1");
	command.print_key("nuked", KeyMetadata(), "11:2,delete.1,two.1");
	command.print_key(".membership/peer", KeyMetadata(), "11:1,two.1");

	string expected = R"("one":"11:1,two.1",
"nuked":"11:2,delete.1,two.1",
)";
	assertEquals(expected, stream.writeBuffer() );
}

TEST_CASE( "ListKeysCommandTest/testAll", "[unit]" )
{
	MockStore store;
	StringByteStream stream;
	ListKeysCommand command(store);
	command.setWriter(&stream);
	command.params.all = true;

	command.print_key("one", KeyMetadata(), "11:1,two.1");
	command.print_key("nuked", KeyMetadata(), "11:2,delete.1,two.1");
	command.print_key(".membership/peer", KeyMetadata(), "11:1,two.1");

	string expected = R"("one":"11:1,two.1",
".membership/peer":"11:1,two.1",
)";
	assertEquals( expected, stream.writeBuffer() );
}
