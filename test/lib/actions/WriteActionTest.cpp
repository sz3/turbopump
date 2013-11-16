#include "unittest.h"

#include "WriteAction.h"

#include "common/DataBuffer.h"
#include "data_store/IDataStoreReader.h"
#include "data_store/LocalDataStore.h"
#include "util/CallHistory.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>

using std::string;

class TestableLocalDataStore : public LocalDataStore
{
public:
	using LocalDataStore::_store;
};

namespace {
	CallHistory _history;
}

TEST_CASE( "WriteActionTest/testDefault", "default" )
{
	TestableLocalDataStore dataStore;
	{
		WriteAction action(dataStore, [&](string filename, IDataStoreReader::ptr){ _history.call("onCommit", filename); });
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
	assertEquals( "0123456789abcde", *dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

TEST_CASE( "WriteActionTest/testDestructorCleanup", "default" )
{
	_history.clear();
	TestableLocalDataStore dataStore;
	{
		WriteAction action(dataStore, [&](string filename, IDataStoreReader::ptr){ _history.call("onCommit", filename); });

		std::map<string,string> params;
		params["name"] = "foobar.txt";
		action.setParams(params);

		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
	}
	assertEquals( "0123456789", *dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

TEST_CASE( "WriteActionTest/testZeroByteWrite", "default" )
{
	_history.clear();
	TestableLocalDataStore dataStore;
	{
		WriteAction action(dataStore, [&](string filename, IDataStoreReader::ptr){ _history.call("onCommit", filename); });

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
	assertEquals( "", *dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

