/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteCommand.h"

#include "WriteInstructions.h"
#include "data_store/IDataStoreReader.h"
#include "mock/MockDataStore.h"
#include "util/CallHistory.h"
#include <string>
using std::string;

namespace {
	CallHistory _history;

	class TestableWriteCommand : public WriteCommand
	{
	public:
		TestableWriteCommand(IDataStore& dataStore, std::function<void(WriteInstructions&, IDataStoreReader::ptr)> onCommit=NULL)
			: WriteCommand(dataStore, onCommit)
		{}

		using WriteCommand::commit;
		using WriteCommand::flush;

		using WriteCommand::_bytesSinceLastFlush;
		using WriteCommand::_instructions;
	};
}

TEST_CASE( "WriteCommandTest/testDefault", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		auto callback = [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name, ins.mirror, ins.copies, "["+ins.version+"]", ins.isComplete); };
		TestableWriteCommand command(dataStore, callback);
		command._instructions.name = "foobar.txt";

		assertTrue( command.run("0123456789", 10) );
		assertEquals( "", _history.calls() );

		assertTrue( command.run("abcde", 5) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
		assertFalse( command.run("closed", 6) );
	}
	assertEquals( "0123456789abcde", dataStore._store["foobar.txt"] );
	assertEquals( "Writer::setOffset(0)|Writer::write(foobar.txt,0123456789)|Writer::write(foobar.txt,abcde)|Writer::commit(foobar.txt,3)", dataStore._history.calls() );
	assertEquals( "onCommit(foobar.txt,0,3,[1,mockReaderVersion:1],1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testExtraParams", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		auto callback = [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name, ins.mirror, ins.copies, "["+ins.version+"]", ins.source, ins.isComplete); };
		TestableWriteCommand command(dataStore, callback);
		command._instructions.name = "foobar.txt";
		command._instructions.copies = 5;
		command._instructions.mirror = 3;
		command._instructions.version = "v1";
		command._instructions.source = "someguy";
		command._instructions.offset = 20;

		assertTrue( command.run("0123456789", 10) );
		assertEquals( "", _history.calls() );

		assertTrue( command.run() );
		assertTrue( command.finished() );
	}
	assertEquals( "0123456789", dataStore._store["foobar.txt"] );
	assertEquals( "Writer::setOffset(20)|Writer::write(foobar.txt,0123456789)|Writer::commit(foobar.txt,5)", dataStore._history.calls() );
	assertEquals( "onCommit(foobar.txt,3,5,[v1],someguy,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testDestructorCleanup", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name); });
		command._instructions.name = "foobar.txt";
		assertTrue( command.run("0123456789", 10) );
	}
	assertEquals( "0123456789", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testFlush", "default" )
{
	_history.clear();
	MockDataStore dataStore;

	TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name, ins.offset, "["+ins.version+"]"); });
	command._instructions.name = "foobar.txt";

	assertTrue( command.run("0123456789", 10) );
	assertEquals( 10, command._bytesSinceLastFlush );
	assertEquals( "", command._instructions.version );
	assertTrue( command.flush() );
	assertEquals( 0, command._bytesSinceLastFlush );
	assertEquals( "1,mockReaderVersion:1", command._instructions.version );
	assertEquals( 10, command._instructions.offset );

	assertTrue( command.run("abcdef", 6) );
	assertTrue( command.flush() );
	assertEquals( 16, command._instructions.offset );

	assertEquals( "0123456789abcdef", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt,0,[1,mockReaderVersion:1])"
				  "|onCommit(foobar.txt,10,[1,mockReaderVersion:1])", _history.calls() );
	assertEquals( "Writer::setOffset(0)|Writer::write(foobar.txt,0123456789)"
				  "|Writer::commit(foobar.txt,3)|Writer::write(foobar.txt,abcdef)|Writer::commit(foobar.txt,3)", dataStore._history.calls() );
}

TEST_CASE( "WriteCommandTest/testFlush.VersionSpecified", "default" )
{
	_history.clear();
	MockDataStore dataStore;

	TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name, ins.offset, "["+ins.version+"]"); });
	command._instructions.name = "foobar.txt";
	command._instructions.version = "version1";

	assertTrue( command.run("0123456789", 10) );
	assertTrue( command.flush() );
	assertTrue( command.run("abcdef", 6) );
	assertTrue( command.flush() );

	assertEquals( "0123456789abcdef", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt,0,[version1])"
				  "|onCommit(foobar.txt,10,[version1])", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testFlush.NoCallback", "default" )
{
	_history.clear();
	MockDataStore dataStore;

	TestableWriteCommand command(dataStore);
	command._instructions.name = "foobar.txt";
	command._instructions.version = "version1";

	assertTrue( command.run("0123456789", 10) );
	assertTrue( command.flush() );
	assertTrue( command.run("abcdef", 6) );
	assertTrue( command.flush() );

	assertEquals( "0123456789abcdef", dataStore._store["foobar.txt"] );
	assertEquals( 16, command._instructions.offset );
}

TEST_CASE( "WriteCommandTest/testBadName", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		WriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name); });

		assertFalse( command.run() );
		assertTrue( command.finished() );
		assertEquals( 400, command.status().integer() );
	}
	assertEquals( "", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testZeroByteWrite", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr){ _history.call("onCommit", ins.name); });
		command._instructions.name = "foobar.txt";

		assertTrue( command.run(NULL, 0) );
		assertFalse( command.finished() );
		assertTrue( command.run(NULL, 0) );
		assertTrue( command.finished() );
		assertFalse( command.run(NULL, 0) );
	}
	assertEquals( "", dataStore._store["foobar.txt"] );
	assertEquals( "onCommit(foobar.txt)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBigWrite", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr reader){ _history.call("onCommit", ins.name, ins.offset, reader->size(), ins.isComplete); });
		command._instructions.name = "bigfile.txt";

		std::string buff;
		buff.resize(1024, 'a');
		for (unsigned i = 0; i < 65; ++i)
			assertTrue( command.run(buff.data(), buff.size()) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
	}
	assertEquals( (65*1024), dataStore._store["bigfile.txt"].size() );
	assertEquals( "onCommit(bigfile.txt,0,65536,0)|onCommit(bigfile.txt,65536,66560,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBigWrite.Exact", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr reader){ _history.call("onCommit", ins.name, reader->size(), ins.isComplete); });
		command._instructions.name = "bigfile.txt";

		std::string buff;
		buff.resize(1024, 'a');
		for (unsigned i = 0; i < 64; ++i)
			assertTrue( command.run(buff.data(), buff.size()) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
	}
	assertEquals( 65536, dataStore._store["bigfile.txt"].size() );
	assertEquals( "onCommit(bigfile.txt,65536,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBigWrite.Split", "default" )
{
	_history.clear();
	MockDataStore dataStore;
	{
		TestableWriteCommand command(dataStore, [&](WriteInstructions& ins, IDataStoreReader::ptr reader){ _history.call("onCommit", ins.name, ins.offset, reader->size(), ins.isComplete); });
		command._instructions.name = "bigfile.txt";

		std::string buff;
		buff.resize(1000, 'a');
		for (unsigned i = 0; i < 66; ++i)
			assertTrue( command.run(buff.data(), buff.size()) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
	}
	assertEquals( 66000, dataStore._store["bigfile.txt"].size() );
	assertEquals( "onCommit(bigfile.txt,0,65536,0)|onCommit(bigfile.txt,65536,66000,1)", _history.calls() );
}
