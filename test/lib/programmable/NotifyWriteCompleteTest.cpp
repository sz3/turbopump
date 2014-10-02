/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "NotifyWriteComplete.h"

#include "api/WriteInstructions.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockMembership.h"
#include "mock/MockMessageSender.h"
#include <string>

using std::string;

TEST_CASE( "NotifyWriteCompleteTest/testNotLastCopy", "[unit]" )
{
	MockMembership membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 0, 2);
	command.run(params, NULL);

	assertEquals( "", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testNoExtraMirror", "[unit]" )
{
	MockMembership membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 2, 2);
	command.run(params, NULL);

	assertEquals( "", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testExtraMirrorNotAMember", "[unit]" )
{
	MockMembership membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 2, 2);
	params.source = "bob";
	command.run(params, NULL);

	assertEquals( "lookup(bob)", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testDropExtraMirror", "[unit]" )
{
	MockMembership membership;
	membership.addIp("peer", "peer");
	membership._history.clear();
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	WriteInstructions params("myfile", "v1", 3, 2);
	params.source = "peer";
	IDataStoreReader::ptr reader(new MockDataStore::Reader("data"));
	command.run(params, reader);

	assertEquals( "lookup(peer)", membership._history.calls() );
	assertEquals( "acknowledgeWrite(peer,myfile,v1,4)", messenger._history.calls() );
}
