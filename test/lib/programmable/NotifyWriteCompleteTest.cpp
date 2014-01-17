/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "NotifyWriteComplete.h"

#include "common/KeyMetadata.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockMembership.h"
#include "mock/MockMessageSender.h"
#include <string>

using std::string;

TEST_CASE( "NotifyWriteCompleteTest/testNotLastCopy", "[unit]" )
{
	MockMembership membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	KeyMetadata md("myfile", 0, 2);
	command.run(md, NULL);

	assertEquals( "", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testNoExtraMirror", "[unit]" )
{
	MockMembership membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	KeyMetadata md("myfile", 2, 2);
	command.run(md, NULL);

	assertEquals( "", membership._history.calls() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "NotifyWriteCompleteTest/testExtraMirrorNotAMember", "[unit]" )
{
	MockMembership membership;
	MockMessageSender messenger;
	NotifyWriteComplete command(membership, messenger);

	KeyMetadata md("myfile", 2, 2);
	md.source = "bob";
	command.run(md, NULL);

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

	KeyMetadata md("myfile", 3, 2);
	md.source = "peer";
	command.run(md, NULL);

	assertEquals( "lookup(peer)", membership._history.calls() );
	assertEquals( "dropKey(peer,myfile)", messenger._history.calls() );
}
