/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "OfferWriteCommand.h"
#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockMessageSender.h"

TEST_CASE( "OfferWriteCommandTest/testRun.AlreadyHaveKey", "[unit]" )
{
	MockDataStore store;
	store._store["foo"] = "bar";
	MockMessageSender messenger;
	OfferWriteCommand command(store, messenger);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";
	command.params.version = "28";
	command.params.source = "sangra";

	assertFalse( command.run() );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "OfferWriteCommandTest/testRun.NeedKey", "[unit]" )
{
	MockDataStore store;
	MockMessageSender messenger;
	OfferWriteCommand command(store, messenger);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";
	command.params.version = "28";
	command.params.source = "sangra";

	assertTrue( command.run() );
	assertEquals( "demandWrite(peer,foo,28,sangra)", messenger._history.calls() );
}
