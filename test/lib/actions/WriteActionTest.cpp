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

	class TestableWriteAction : public WriteAction
	{
	public:
		TestableWriteAction(IDataStore& dataStore, std::function<void(WriteParams, IDataStoreReader::ptr)> onCommit=NULL)
			: WriteAction(dataStore, onCommit)
		{}

		using WriteAction::commit;
		using WriteAction::flush;

		using WriteAction::_bytesSinceLastFlush;
		using WriteAction::_params;
	};
}

TEST_CASE( "WriteActionTest/testDefault", "default" )
{
	_history.clear();
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
	assertEquals( "Writer::setOffset(0)|Writer::write(foobar.txt,0123456789)|Writer::write(foobar.txt,abcde)|Writer::commit(foobar.txt,3)", dataStore._history.calls() );
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
		params["offset"] = "20";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		assertTrue( action.run(DataBuffer("0123456789", 10)) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}
	assertEquals( "0123456789", dataStore._store["foobar.txt"] );
	assertEquals( "Writer::setOffset(20)|Writer::write(foobar.txt,0123456789)|Writer::commit(foobar.txt,5)", dataStore._history.calls() );
	assertEquals( "onCommit(foobar.txt,3,5,[v1],someguy)", _history.calls() );
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

TEST_CASE( "WriteActionTest/testFlush", "default" )
{
	_history.clear();
	MockDataStore dataStore;

	TestableWriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.offset, "["+params.version+"]"); });

	std::map<string,string> params;
	params["name"] = "foobar.txt";
	action.setParams(params);

	assertTrue( action.run(DataBuffer("0123456789", 10)) );
	assertEquals( 10, action._bytesSinceLastFlush );
	assertEquals( "", action._params.version );
	assertTrue( action.flush() );
	assertEquals( 0, action._bytesSinceLastFlush );
	assertEquals( "1,mockReaderVersion:1", action._params.version );
	assertEquals( 10, action._params.offset );

	assertTrue( action.run(DataBuffer("abcdef", 6)) );
	assertTrue( action.flush() );
	assertEquals( 16, action._params.offset );

	assertEquals( "0123456789abcdef", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt,0,[1,mockReaderVersion:1])"
				  "|onCommit(foobar.txt,10,[1,mockReaderVersion:1])", _history.calls() );
	assertEquals( "Writer::setOffset(0)|Writer::write(foobar.txt,0123456789)"
				  "|Writer::commit(foobar.txt,3)|Writer::write(foobar.txt,abcdef)|Writer::commit(foobar.txt,3)", dataStore._history.calls() );
}

TEST_CASE( "WriteActionTest/testFlush.VersionSpecified", "default" )
{
	_history.clear();
	MockDataStore dataStore;

	TestableWriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.offset, "["+params.version+"]"); });

	std::map<string,string> params;
	params["name"] = "foobar.txt";
	params["v"] = "version1";
	action.setParams(params);

	assertTrue( action.run(DataBuffer("0123456789", 10)) );
	assertTrue( action.flush() );
	assertTrue( action.run(DataBuffer("abcdef", 6)) );
	assertTrue( action.flush() );

	assertEquals( "0123456789abcdef", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt,0,[version1])"
				  "|onCommit(foobar.txt,10,[version1])", _history.calls() );
}

TEST_CASE( "WriteActionTest/testFlush.NoCallback", "default" )
{
	_history.clear();
	MockDataStore dataStore;

	TestableWriteAction action(dataStore);

	std::map<string,string> params;
	params["name"] = "foobar.txt";
	params["v"] = "version1";
	action.setParams(params);

	assertTrue( action.run(DataBuffer("0123456789", 10)) );
	assertTrue( action.flush() );
	assertTrue( action.run(DataBuffer("abcdef", 6)) );
	assertTrue( action.flush() );

	assertEquals( "0123456789abcdef", dataStore._store["foobar.txt"] );
	assertEquals( 16, action._params.offset );
}

TEST_CASE( "WriteActionTest/testBadName", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr){ _history.call("onCommit", params.filename, params.mirror, params.totalCopies, "["+params.version+"]"); });
		assertFalse( action.good() );
	}
	assertEquals( "", _history.calls() );
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

TEST_CASE( "WriteActionTest/testBigWrite", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr reader){ _history.call("onCommit", params.filename, params.offset, reader->size()); });
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "bigfile.txt";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		std::string buff;
		buff.resize(1024, 'a');
		for (unsigned i = 0; i < 65; ++i)
			assertTrue( action.run(DataBuffer(buff.data(), buff.size())) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}
	assertEquals( (65*1024), dataStore._store["bigfile.txt"].size() );
	assertEquals( "onCommit(bigfile.txt,0,65536)|onCommit(bigfile.txt,65536,66560)", _history.calls() );
}

TEST_CASE( "WriteActionTest/testBigWrite.Exact", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr reader){ _history.call("onCommit", params.filename, reader->size()); });
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "bigfile.txt";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		std::string buff;
		buff.resize(1024, 'a');
		for (unsigned i = 0; i < 64; ++i)
			assertTrue( action.run(DataBuffer(buff.data(), buff.size())) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}
	assertEquals( 65536, dataStore._store["bigfile.txt"].size() );
	assertEquals( "onCommit(bigfile.txt,65536)", _history.calls() );
}


TEST_CASE( "WriteActionTest/testBigWrite.Split", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteAction action(dataStore, [&](WriteParams params, IDataStoreReader::ptr reader){ _history.call("onCommit", params.filename, params.offset, reader->size()); });
		assertFalse( action.good() );

		std::map<string,string> params;
		params["name"] = "bigfile.txt";
		action.setParams(params);
		assertTrue( action.good() );
		assertEquals( "", _history.calls() );

		std::string buff;
		buff.resize(1000, 'a');
		for (unsigned i = 0; i < 66; ++i)
			assertTrue( action.run(DataBuffer(buff.data(), buff.size())) );
		assertTrue( action.run(DataBuffer::Null()) );
		assertTrue( action.finished() );
	}
	assertEquals( 66000, dataStore._store["bigfile.txt"].size() );
	assertEquals( "onCommit(bigfile.txt,0,65536)|onCommit(bigfile.txt,65536,66000)", _history.calls() );
}
