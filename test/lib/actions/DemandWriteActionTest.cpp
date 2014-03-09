/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DemandWriteAction.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockSkewCorrector.h"

#include <map>
using std::string;

namespace {
	CallHistory _history;
}

TEST_CASE( "DemandWriteActionTest/testRun", "default" )
{
	Peer peer("fooid");
	MockSkewCorrector corrector;
	DemandWriteAction action(peer, corrector);

	std::map<string,string> params;
	params["name"] = "foo";
	params["v"] = "42";
	params["source"] = "sauce";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "sendKey(fooid,foo,42,sauce)", corrector._history.calls() );
}

TEST_CASE( "DemandWriteActionTest/testReject", "default" )
{
	Peer peer("fooid");
	MockSkewCorrector corrector;
	DemandWriteAction action(peer, corrector);

	assertFalse( action.run(DataBuffer::Null()) );
	assertEquals( "", corrector._history.calls() );
}

TEST_CASE( "DemandWriteActionTest/testDefaults", "default" )
{
	Peer peer("fooid");
	MockSkewCorrector corrector;
	DemandWriteAction action(peer, corrector);

	std::map<string,string> params;
	params["name"] = "foo";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "sendKey(fooid,foo,,)", corrector._history.calls() );
}
