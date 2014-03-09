/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "HealKeyAction.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockSkewCorrector.h"
using std::string;

TEST_CASE( "HealKeyActionTest/testOne", "default" )
{
	Peer peer("fooid");
	MockSkewCorrector corrector;
	HealKeyAction action(peer, corrector);

	std::map<string,string> params;
	params["tree"] = "oak";
	params["n"] = "5";
	params["key"] = "1234";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "healKey(fooid,oak,1234)", corrector._history.calls() );
}

