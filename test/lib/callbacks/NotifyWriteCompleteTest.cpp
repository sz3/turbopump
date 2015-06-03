/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "NotifyWriteComplete.h"

#include "api/WriteInstructions.h"
#include "common/KeyMetadata.h"
#include "membership/Peer.h"
#include "mock/MockKnownPeers.h"
#include "mock/MockMessageSender.h"
#include "storage/readstream.h"
#include "storage/StringReader.h"
#include <string>

using std::string;

TEST_CASE( "NotifyWriteCompleteTest/testNotLastCopy", "[unit]" )
{
	MockKnownPeers membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 0, 2);
	readstream reader(NULL, KeyMetadata());
	command.run(params, reader);

	assertEquals( "", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testNoExtraMirror", "[unit]" )
{
	MockKnownPeers membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 2, 2);
	readstream reader(NULL, KeyMetadata());
	command.run(params, reader);

	assertEquals( "", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testExtraMirrorNotAMember", "[unit]" )
{
	MockKnownPeers membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 2, 2);
	params.source = "bob";
	readstream reader(NULL, KeyMetadata());
	command.run(params, reader);

	assertEquals( "lookup(bob)", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testDropExtraMirror", "[unit]" )
{
	MockKnownPeers membership;
	membership.update("peer");
	membership._history.clear();
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 3, 2);
	params.source = "peer";
	readstream reader(new StringReader("data"), KeyMetadata());
	command.run(params, reader);

	assertEquals( "lookup(peer)", membership._history.calls() );
	assertEquals( "acknowledgeWrite(peer,myfile,v1,4)", messenger._history.calls() );
}
