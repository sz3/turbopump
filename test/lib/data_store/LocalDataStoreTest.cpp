/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataEntry.h"
#include "LocalDataStore.h"
#include "IDataStoreReader.h"
#include "IDataStoreWriter.h"
#include "socket/IByteStream.h"
#include <string>

using std::string;

class TestableLocalDataStore : public LocalDataStore
{
public:
	using LocalDataStore::_store;
	using LocalDataStore::Reader;
	using LocalDataStore::Writer;
};

class TestableWriter : public TestableLocalDataStore::Writer
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

TEST_CASE( "LocalDataStoreTest/testWrite", "default" )
{
	TestableLocalDataStore dataStore;

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

TEST_CASE( "LocalDataStoreTest/testOverwrite", "default" )
{
	TestableLocalDataStore dataStore;
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

TEST_CASE( "LocalDataStoreTest/testRead", "default" )
{
	TestableLocalDataStore dataStore;
	dataStore._store["foo"].reset(new DataEntry({"readme"}));

	StringBackedByteStream stream;
	IDataStoreReader::ptr reader = dataStore.read("foo");
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

TEST_CASE( "LocalDataStoreTest/testConcurrentRead", "default" )
{
	TestableLocalDataStore dataStore;
	dataStore._store["foo"].reset(new DataEntry({"readme"}));

	StringBackedByteStream stream;
	IDataStoreReader::ptr reader = dataStore.read("foo");

	IDataStoreWriter::ptr writer = dataStore.write("foo");
	assertTrue( writer->write("0123456789", 10) );
	assertTrue( writer->commit() );

	IDataStoreReader::ptr lateReader = dataStore.read("foo");
	assertEquals( 10, lateReader->read(stream) );
	assertEquals( "0123456789", stream._buffer );

	// now, the original reader
	assertEquals( 6, reader->read(stream) );
	assertEquals( "readme", stream._buffer );
}
