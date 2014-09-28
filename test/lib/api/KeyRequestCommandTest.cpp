/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "KeyRequestCommand.h"
#include "membership/Peer.h"
#include "mock/MockSkewCorrector.h"

TEST_CASE( "KeyRequestCommandTest/testDefault", "[unit]" )
{
	MockSkewCorrector corrector;
	KeyRequestCommand command(corrector);

	command.setPeer( std::shared_ptr<Peer>(new Peer("peer")) );
	command.params.id = "oak";
	command.params.mirrors = 4;
	command.params.first = 52;
	command.params.last = 1337;

	assertTrue( command.run() );
	assertEquals( "pushKeyRange(peer,oak,4,52,1337,)", corrector._history.calls() );
}
