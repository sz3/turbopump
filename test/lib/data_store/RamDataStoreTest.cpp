/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataEntry.h"
#include "RamDataStore.h"
#include "IDataStoreReader.h"
#include "IDataStoreWriter.h"
#include "socket/IByteStream.h"
#include <string>

using std::string;
using std::vector;

class TestableRamDataStore : public RamDataStore
{
public:
	using RamDataStore::_store;
	using RamDataStore::Reader;
	using RamDataStore::Writer;
};

class TestableWriter : public TestableRamDataStore::Writer
{
public:
	const std::string& filename() const
	{
		return _filename;
	}

	const DataEntry& data() const
	{
		return _data;
	}
};

TEST_CASE( "RamDataStoreTest/testWrite", "[unit]" )
{
	TestableRamDataStore dataStore;

	IDataStoreWriter::ptr writer = dataStore.write("foo");
	TestableWriter* testView = (TestableWriter*)writer.get();
	assertEquals( "foo", testView->filename() );

	assertTrue( writer->write("012345", 6) );
	assertEquals( "012345", testView->data().data );

	assertTrue( writer->write("6789", 4) );
	assertEquals( "0123456789", testView->data().data );

	assertTrue( dataStore._store.find("foo") == dataStore._store.end() );
	assertTrue( writer->commit() );

	assertEquals( "0123456789", dataStore._store["foo"]->data );
	assertEquals( "", testView->data().data ); // got std::move'd
}

TEST_CASE( "RamDataStoreTest/testOverwrite", "[unit]" )
{
	TestableRamDataStore dataStore;
	{
		IDataStoreWriter::ptr writer = dataStore.write("foo");
		assertTrue( writer->write("012345", 6) );
		assertTrue( writer->commit() );
		assertEquals( "012345", dataStore._store["foo"]->data );
	}
	{
		IDataStoreWriter::ptr writer = dataStore.write("foo");
		assertTrue( writer->write("different!", 10) );
		assertTrue( writer->commit() );
		assertEquals( "different!", dataStore._store["foo"]->data );
	}
}

class StringBackedByteStream : public IByteStream
{
public:
	unsigned maxPacketLength() const
	{
		return 1000;
	}

	int read(char* buffer, unsigned length)
	{
		return 0;
	}

	int write(const char* buffer, unsigned length)
	{
		_buffer = string(buffer, length);
		return length;
	}

public:
	string _buffer;
};

TEST_CASE( "RamDataStoreTest/testRead", "[unit]" )
{
	TestableRamDataStore dataStore;
	dataStore._store["foo"].reset(new DataEntry({"readme"}));

	vector<IDataStoreReader::ptr> readerList = dataStore.read("foo");
	assertEquals( 1, readerList.size() );

	IDataStoreReader::ptr reader = dataStore.read("foo", "");
	assertEquals( reader.get(), readerList.front().get() );

	StringBackedByteStream stream;
	assertEquals( 6, reader->read(stream) );
	assertEquals( "readme", stream._buffer );

	assertEquals( 0, reader->read(stream) );

	assertTrue( reader->seek(1) );
	assertEquals( 5, reader->read(stream) );
	assertEquals( "eadme", stream._buffer );

	assertFalse( reader->seek(-1) );
	assertFalse( reader->seek(1000) );
	assertTrue( reader->seek(4) );
	assertEquals( 2, reader->read(stream) );
	assertEquals( "me", stream._buffer );
}

TEST_CASE( "RamDataStoreTest/testRead.Concurrent", "[unit]" )
{
	TestableRamDataStore dataStore;
	dataStore._store["foo"].reset(new DataEntry({"readme"}));

	StringBackedByteStream stream;
	IDataStoreReader::ptr reader = dataStore.read("foo", "");

	IDataStoreWriter::ptr writer = dataStore.write("foo");
	assertTrue( writer->write("0123456789", 10) );
	assertTrue( writer->commit() );

	vector<IDataStoreReader::ptr> lateReaderList = dataStore.read("foo");
	assertEquals( 1, lateReaderList.size() );

	IDataStoreReader::ptr lateReader = dataStore.read("foo", "");
	assertEquals( lateReader.get(), lateReaderList.front().get() );

	assertEquals( 10, lateReader->read(stream) );
	assertEquals( "0123456789", stream._buffer );

	// now, the original reader
	assertEquals( 6, reader->read(stream) );
	assertEquals( "readme", stream._buffer );
}

TEST_CASE( "RamDataStoreTest/testReport", "[unit]" )
{
	TestableRamDataStore dataStore;
	dataStore._store["foo"].reset(new DataEntry({"bytes"}));
	dataStore._store["foobar"].reset(new DataEntry({"bytes"}));
	dataStore._store["bar"].reset(new DataEntry({"bytes"}));

	StringBackedByteStream stream;
	dataStore.report(stream);

	// TODO: have a proper ByteStream mock...
	assertEquals( "\n(foobar)=>5", stream._buffer );
}

TEST_CASE( "RamDataStoreTest/testReport.Exclude", "[unit]" )
{
	TestableRamDataStore dataStore;
	dataStore._store["foo"].reset(new DataEntry({"bytes"}));
	dataStore._store["foobar"].reset(new DataEntry({"bytes"}));
	dataStore._store["bar"].reset(new DataEntry({"bytes"}));

	StringBackedByteStream stream;
	dataStore.report(stream, "foo");

	assertEquals( "(bar)=>5", stream._buffer );
}
