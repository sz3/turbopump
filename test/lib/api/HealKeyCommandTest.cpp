/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "HealKeyCommand.h"
#include "membership/Peer.h"
#include "mock/MockSkewCorrector.h"

TEST_CASE( "HealKeyCommandTest/testHeal", "[unit]" )
{
	MockSkewCorrector corrector;
	HealKeyCommand command(corrector);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.id = "oak";
	command.params.mirrors = 5;
	command.params.key = 1234;

	assertTrue( command.run() );
	assertEquals( "healKey(peer,oak,1234)", corrector._history.calls() );
}

