/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DemandWriteCommand.h"
#include "membership/Peer.h"
#include "mock/MockSkewCorrector.h"

TEST_CASE( "DemandWriteCommandTest/testRun", "[unit]" )
{
	MockSkewCorrector corrector;
	DemandWriteCommand command(corrector);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";
	command.params.version = "28";
	command.params.source = "sangra";

	assertTrue( command.run() );
	assertEquals( "sendKey(peer,foo,28,sangra)", corrector._history.calls() );
}

TEST_CASE( "DemandWriteCommandTest/testReject", "[unit]" )
{
	MockSkewCorrector corrector;
	DemandWriteCommand command(corrector);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );

	assertFalse( command.run() );
	assertEquals( "", corrector._history.calls() );
}

TEST_CASE( "DemandWriteCommandTest/testDefaults", "[unit]" )
{
	MockSkewCorrector corrector;
	DemandWriteCommand command(corrector);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.name = "foo";

	assertTrue( command.run() );
	assertEquals( "sendKey(peer,foo,,)", corrector._history.calls() );
}
