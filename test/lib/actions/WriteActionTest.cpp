/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteAction.h"

#include "actions/WriteParams.h"
#include "common/DataBuffer.h"
#include "data_store/IDataStoreReader.h"
#include "mock/MockDataStore.h"
#include "util/CallHistory.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>

using std::string;

namespace {
	CallHistory _history;
}

TEST_CASE( "WriteActionTest/testDefault", "default" )
{
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.mirror, params.totalCopies, "["+params.version+"]"); });
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
		assertTrue( action.run(DataBuffer("abcde", 5)) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
		assertFalse( action.run(DataBuffer("closed", 6)) );
	}
	assertEquals( "0123456789abcde", dataStore._store["foobar.txt"] );
	assertEquals( "Writer::write(foobar.txt,0123456789)|Writer::write(foobar.txt,abcde)|Writer::commit(foobar.txt,3)", dataStore._history.calls() );
	assertEquals( "onCommit(foobar.txt,0,3,[1,mockReaderVersion:1])", _history.calls() );
}

TEST_CASE( "WriteActionTest/testExtraParams", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		auto callback = [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.mirror, params.totalCopies, "["+params.version+"]", params.source); };
		WriteAction action(dataStore, callback);
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		params["n"] = "5";
		params["i"] = "3";
		params["v"] = "v1";
		params["source"] = "someguy";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}
	assertEquals( "0123456789", dataStore._store["foobar.txt"] );
	assertEquals( "Writer::write(foobar.txt,0123456789)|Writer::commit(foobar.txt,5)", dataStore._history.calls() );
	assertEquals( "onCommit(foobar.txt,3,5,[1,mockReaderVersion:1],someguy)", _history.calls() );
}

TEST_CASE( "WriteActionTest/testDestructorCleanup", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename); });

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		action.setParams(params);

		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
	}
	assertEquals( "0123456789", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

TEST_CASE( "WriteActionTest/testZeroByteWrite", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename); });

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		action.setParams(params);

		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer::Null()) );
		assertFalse( action.finished() );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
		assertFalse( action.run(DataBuffer::Null()) );
	}
	assertEquals( "", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

