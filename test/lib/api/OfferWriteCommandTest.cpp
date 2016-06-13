/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "OfferWriteCommand.h"
#include "membership/Peer.h"
#include "mock/MockMessageSender.h"
#include "mock/MockStore.h"

TEST_CASE( "OfferWriteCommandTest/testRun.AlreadyHaveKey", "[unit]" )
{
	MockStore store;
	store._reads["foo"] = "bar";
	MockMessageSender messenger;
	OfferWriteCommand command(store, messenger);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";
	command.params.version = "28";
	command.params.size = 3;
	command.params.source = "sangra";

	assertFalse( command.run() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "OfferWriteCommandTest/testRun.HavePartOfKey", "[unit]" )
{
	MockStore store;
	store._reads["foo"] = "foo";
	MockMessageSender messenger;
	OfferWriteCommand command(store, messenger);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";
	command.params.version = "28";
	command.params.size = 1234; // longer than "foo"
	command.params.source = "sangra";

	assertTrue( command.run() );
	assertEquals( "demandWrite(peer,foo,28,3,sangra)", messenger._history.calls() );
}

TEST_CASE( "OfferWriteCommandTest/testRun.NeedKey", "[unit]" )
{
	MockStore store;
	store._exists = false;
	MockMessageSender messenger;
	OfferWriteCommand command(store, messenger);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";
	command.params.version = "28";
	command.params.size = 1234;
	command.params.source = "sangra";

	assertTrue( command.run() );
	assertEquals( "demandWrite(peer,foo,28,0,sangra)", messenger._history.calls() );
}
