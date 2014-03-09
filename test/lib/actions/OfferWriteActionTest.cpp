/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "OfferWriteAction.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockMessageSender.h"

#include <map>
using std::string;

namespace {
	CallHistory _history;
}

TEST_CASE( "OfferWriteActionTest/testRun.AlreadyHaveKey", "default" )
{
	Peer peer("fooid");
	MockDataStore store;
	store._store["foo"] = "bar";
	MockMessageSender messenger;
	OfferWriteAction action(peer, store, messenger);

	std::map<string,string> params;
	params["name"] = "foo";
	params["v"] = "42";
	params["source"] = "sauce";
	action.setParams(params);

	assertFalse( action.run(DataBuffer::Null()) );
	assertEquals( "", messenger._history.calls() );
}

TEST_CASE( "OfferWriteActionTest/testRun.NeedKey", "default" )
{
	Peer peer("fooid");
	MockDataStore store;
	MockMessageSender messenger;
	OfferWriteAction action(peer, store, messenger);

	std::map<string,string> params;
	params["name"] = "foo";
	params["v"] = "42";
	params["source"] = "sauce";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "demandWrite(fooid,foo,42,sauce)", messenger._history.calls() );
}
