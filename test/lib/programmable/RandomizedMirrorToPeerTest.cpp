/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "RandomizedMirrorToPeer.h"

#include "actions/WriteParams.h"
#include "data_store/IDataStoreReader.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockDataStore.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"

#include "util/CallHistory.h"
#include <string>

using std::string;

namespace {
	CallHistory _history;
}

TEST_CASE( "RandomizedMirrorToPeerTest/testDefault", "[unit]" )
{
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	MockPeerTracker peers;
	RandomizedMirrorToPeer command(membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run({"file", 123, 456}, reader) );

	assertEquals( "addIp(1.2.3.4,dude)|randomPeer()", membership._history.calls() );
	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=123 n=456|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
}

