/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteCommand.h"

#include "WriteInstructions.h"
#include "common/WallClock.h"
#include "storage/readstream.h"

#include "mock/MockStore.h"
#include "mock/MockStoreWriter.h"
#include "util/CallHistory.h"
#include <string>
using std::string;

namespace {
	CallHistory _history;

	class TestableWriteCommand : public WriteCommand
	{
	public:
		TestableWriteCommand(IStore& store, std::function<void(WriteInstructions&, readstream&)> onCommit=NULL)
			: WriteCommand(store, onCommit)
		{}

		using WriteCommand::commit;
		using WriteCommand::flush;

		using WriteCommand::_bytesSinceLastFlush;
		using WriteCommand::_instructions;
	};
}

TEST_CASE( "WriteCommandTest/testDefault", "[unit]" )
{
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	_history.clear();

	MockStore store;
	store._writer = new MockStoreWriter();
	{
		auto callback = [&](WriteInstructions& ins, readstream& stream){ _history.call("onCommit", ins.name, ins.mirror, ins.copies, "{"+ins.version+"}", stream.size(), ins.isComplete); };
		TestableWriteCommand command(store, callback);
		command._instructions.name = "foobar.txt";

		assertTrue( command.run("0123456789", 10) );
		assertEquals( "", _history.calls() );

		assertTrue( command.run("abcde", 5) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
		assertFalse( command.run("closed", 6) );
		assertEquals( 200, command.status() );
	}
	assertEquals( "write(foobar.txt,,0)", store._history.calls() );
	assertEquals( "write(0123456789)|write(abcde)|flush()|close()|reader()", MockStoreWriter::calls() );
	assertEquals( "onCommit(foobar.txt,0,3,{1,mock.UNIXSECONDS=},15,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBadWriter", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = NULL;
	{
		auto callback = [&](WriteInstructions& ins, readstream& stream){ _history.call("onCommit", ins.name); };
		TestableWriteCommand command(store, callback);
		command._instructions.name = "foobar.txt";

		assertFalse( command.run("0123456789", 10) );
		assertEquals( "500", command.status().str() );
		assertEquals( "", _history.calls() );
	}
	assertEquals( "write(foobar.txt,,0)", store._history.calls() );
	assertEquals( "", MockStoreWriter::calls() );
	assertEquals( "", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testExtraParams", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		auto callback = [&](WriteInstructions& ins, readstream&){ _history.call("onCommit", ins.name, ins.mirror, ins.copies, "{"+ins.version+"}", ins.source, ins.isComplete); };
		TestableWriteCommand command(store, callback);
		command._instructions.name = "foobar.txt";
		command._instructions.copies = 5;
		command._instructions.mirror = 3;
		VectorClock version;
		version.increment("v1");
		command._instructions.version = version.toString();
		command._instructions.source = "someguy";
		command._instructions.offset = 20;

		assertTrue( command.run("0123456789", 10) );
		assertEquals( "", _history.calls() );

		assertTrue( command.run() );
		assertTrue( command.finished() );
		assertEquals( 200, command.status() );
	}
	assertEquals( "write(foobar.txt,1,v1.UNIXSECONDS=,20)", store._history.calls() );
	assertEquals( "write(0123456789)|flush()|close()|reader()", MockStoreWriter::calls() );
	assertEquals( "onCommit(foobar.txt,3,5,{1,v1.UNIXSECONDS=},someguy,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testDestructorNoCommit", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream&){ _history.call("onCommit", ins.name); });
		command._instructions.name = "foobar.txt";
		assertTrue( command.run("0123456789", 10) );
	}
	assertEquals( "write(foobar.txt,,0)", store._history.calls() );
	assertEquals( "write(0123456789)", MockStoreWriter::calls() );
	assertEquals( "", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testFlush", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();

	TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream&){ _history.call("onCommit", ins.name, ins.offset, "{"+ins.version+"}"); });
	command._instructions.name = "foobar.txt";

	assertTrue( command.run("0123456789", 10) );
	assertEquals( 10, command._bytesSinceLastFlush );
	assertEquals( "", command._instructions.version );
	assertTrue( command.flush() );
	assertEquals( 0, command._bytesSinceLastFlush );
	assertEquals( "1,mock.UNIXSECONDS=", command._instructions.version );
	assertEquals( 10, command._instructions.offset );

	assertTrue( command.run("abcdef", 6) );
	assertTrue( command.flush() );
	assertEquals( 16, command._instructions.offset );

	assertEquals( "write(foobar.txt,,0)", store._history.calls() );
	assertEquals( "write(0123456789)|flush()|reader()|write(abcdef)|flush()|reader()", MockStoreWriter::calls() );
	assertEquals( "onCommit(foobar.txt,0,{1,mock.UNIXSECONDS=})"
				  "|onCommit(foobar.txt,10,{1,mock.UNIXSECONDS=})", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testFlush.VersionSpecified", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();

	TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream&){ _history.call("onCommit", ins.name, ins.offset, "["+ins.version+"]"); });
	command._instructions.name = "foobar.txt";
	command._instructions.version = "version1";

	assertTrue( command.run("0123456789", 10) );
	assertTrue( command.flush() );
	assertTrue( command.run("abcdef", 6) );
	assertTrue( command.flush() );

	assertEquals( "write(foobar.txt,version1,0)", store._history.calls() );
	assertEquals( "write(0123456789)|flush()|reader()|write(abcdef)|flush()|reader()", MockStoreWriter::calls() );
	assertEquals( "onCommit(foobar.txt,0,[version1])"
				  "|onCommit(foobar.txt,10,[version1])", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testFlush.NoCallback", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();

	TestableWriteCommand command(store);
	command._instructions.name = "foobar.txt";
	command._instructions.version = "version1";

	assertTrue( command.run("0123456789", 10) );
	assertTrue( command.flush() );
	assertTrue( command.run("abcdef", 6) );
	assertTrue( command.flush() );

	assertEquals( "write(foobar.txt,version1,0)", store._history.calls() );
	assertEquals( 16, command._instructions.offset );
}

TEST_CASE( "WriteCommandTest/testBadName", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		WriteCommand command(store, [&](WriteInstructions& ins, readstream&){ _history.call("onCommit", ins.name); });

		assertFalse( command.run() );
		assertTrue( command.finished() );
		assertEquals( 400, command.status() );
	}
	assertEquals( "", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testZeroByteWrite", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream& reader){ _history.call("onCommit", ins.name, reader.size()); });
		command._instructions.name = "foobar.txt";

		assertTrue( command.run(NULL, 0) );
		assertFalse( command.finished() );
		assertTrue( command.run(NULL, 0) );
		assertTrue( command.finished() );
		assertFalse( command.run(NULL, 0) );
	}
	assertEquals( "write(foobar.txt,,0)", store._history.calls() );
	assertEquals( "onCommit(foobar.txt,0)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBigWrite", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream& reader){ _history.call("onCommit", ins.name, ins.offset, reader.size(), ins.isComplete); });
		command._instructions.name = "bigfile.txt";

		std::string buff;
		buff.resize(1024, 'a');
		for (unsigned i = 0; i < 65; ++i)
			assertTrue( command.run(buff.data(), buff.size()) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
	}
	assertEquals( "write(bigfile.txt,,0)", store._history.calls() );
	assertEquals( "onCommit(bigfile.txt,0,65536,0)|onCommit(bigfile.txt,65536,66560,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBigWrite.Exact", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream& reader){ _history.call("onCommit", ins.name, reader.size(), ins.isComplete); });
		command._instructions.name = "bigfile.txt";

		std::string buff;
		buff.resize(1024, 'a');
		for (unsigned i = 0; i < 64; ++i)
			assertTrue( command.run(buff.data(), buff.size()) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
		assertEquals( 200, command.status() );
	}
	assertEquals( "write(bigfile.txt,,0)", store._history.calls() );
	assertEquals( "onCommit(bigfile.txt,65536,1)", _history.calls() );
}

TEST_CASE( "WriteCommandTest/testBigWrite.Split", "[unit]" )
{
	_history.clear();
	MockStore store;
	store._writer = new MockStoreWriter();
	{
		TestableWriteCommand command(store, [&](WriteInstructions& ins, readstream& reader){ _history.call("onCommit", ins.name, ins.offset, reader.size(), ins.isComplete); });
		command._instructions.name = "bigfile.txt";

		std::string buff;
		buff.resize(1000, 'a');
		for (unsigned i = 0; i < 66; ++i)
			assertTrue( command.run(buff.data(), buff.size()) );
		assertTrue( command.run() );
		assertTrue( command.finished() );
		assertEquals( 200, command.status() );
	}
	assertEquals( "write(bigfile.txt,,0)", store._history.calls() );
	assertEquals( "onCommit(bigfile.txt,0,65536,0)|onCommit(bigfile.txt,65536,66000,1)", _history.calls() );
}
