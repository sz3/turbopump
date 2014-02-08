/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "LocalWriteAction.h"

#include "actions/WriteParams.h"
#include "common/DataBuffer.h"
#include "data_store/IDataStoreReader.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"
#include "util/CallHistory.h"
#include <string>
#include <map>

using std::string;

namespace {
	CallHistory _history;
}

TEST_CASE( "LocalWriteActionTest/testTransientWrite", "[unit]" )
{
	_history.clear();
	MockDataStore transientStore;
	MockDataStore dataStore;
	MockLocateKeys locator;
	locator._mine = false;

	{
		auto fun = [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.mirror, params.totalCopies, params.source); };
		LocalWriteAction action(transientStore, dataStore, locator, fun);
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		params["n"] = "5";
		params["i"] = "3";
		params["source"] = "someguy";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}

	assertEquals( "keyIsMine(foobar.txt,5)", locator._history.calls() );
	assertEquals( "Writer::write(foobar.txt,0123456789)|Writer::commit(foobar.txt,5)", transientStore._history.calls() );
	assertEquals( "", dataStore._history.calls() );
	assertEquals( "0123456789", transientStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt,3,5,someguy)", _history.calls() );
}

TEST_CASE( "LocalWriteActionTest/testPermanentWrite", "[unit]" )
{
	_history.clear();
	MockDataStore transientStore;
	MockDataStore dataStore;
	MockLocateKeys locator;
	locator._mine = true;

	{
		auto fun = [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.mirror, params.totalCopies, params.source); };
		LocalWriteAction action(transientStore, dataStore, locator, fun);
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		params["n"] = "5";
		params["i"] = "3";
		params["source"] = "someguy";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}

	assertEquals( "keyIsMine(foobar.txt,5)", locator._history.calls() );
	assertEquals( "", transientStore._history.calls() );
	assertEquals( "Writer::write(foobar.txt,0123456789)|Writer::commit(foobar.txt,5)", dataStore._history.calls() );
	assertEquals( "0123456789", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt,3,5,someguy)", _history.calls() );
}
