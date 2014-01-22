/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "KeyReqAction.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockSynchronize.h"
using std::map;
using std::string;

TEST_CASE( "KeyReqActionTest/testDefault", "[unit]" )
{
	Peer peer("fooid");
	MockSynchronize sync;
	KeyReqAction action(peer, sync);

	map<string,string> params;
	params["tree"] = "oak";
	params["n"] = "4";
	params["first"] = "52";
	params["last"] = "1337";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "pushKeyRange(fooid,oak,4,52,1337)", sync._history.calls() );
}
