/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataChain.h"
#include "DataEntry.h"
#include "RamDataStore.h"
#include "IDataStoreReader.h"
#include "IDataStoreWriter.h"

#include "common/KeyMetadata.h"
#include "common/MyMemberId.h"
#include "serialize/StringUtil.h"
#include "socket/IByteStream.h"
#include "socket/StringByteStream.h"
#include <string>

using std::string;
using std::vector;

namespace {
	class TestableRamDataStore : public RamDataStore
	{
	public:
		using RamDataStore::_store;
		using RamDataStore::Reader;
		using RamDataStore::Writer;

		void store(const std::string& filename, const std::string& content)
		{
			std::shared_ptr<DataEntry> data(new DataEntry{content});
			_store[filename].storeAsBestVersion(data);
		}
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

	std::ostream& operator<<(std::ostream& outstream, const bounded_version_vector<string>::clock& clock)
	{
		outstream << clock.key << ":" << clock.count;
		return outstream;
	}

	std::string versionStr(const IDataStoreReader::ptr& reader)
	{
		return StringUtil::join( reader->metadata().version.clocks() );
	}
}

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

	assertEquals( "0123456789", dataStore._store["foo"].entries().front()->data );
	assertEquals( "", testView->data().data ); // got std::move'd
}

TEST_CASE( "RamDataStoreTest/testOverwrite", "[unit]" )
{
	TestableRamDataStore dataStore;
	{
		IDataStoreWriter::ptr writer = dataStore.write("foo");
		assertTrue( writer->write("012345", 6) );
		assertTrue( writer->commit() );
		assertEquals( "012345", dataStore._store["foo"].entries().front()->data );
	}
	{
		IDataStoreWriter::ptr writer = dataStore.write("foo");
		assertTrue( writer->write("different!", 10) );
		assertTrue( writer->commit() );
		assertEquals( "different!", dataStore._store["foo"].entries().front()->data );
	}
}

TEST_CASE( "RamDataStoreTest/testRead", "[unit]" )
{
	MyMemberId("me");

	TestableRamDataStore dataStore;
	dataStore.store("foo", "readme");

	vector<IDataStoreReader::ptr> readerList = dataStore.read("foo");
	assertEquals( 1, readerList.size() );
	assertEquals( "me:1", versionStr(readerList.front()) );

	IDataStoreReader::ptr reader = dataStore.read("foo", "badversion");
	assertFalse( reader );

	reader = dataStore.read("foo", "1|me:1");
	assertTrue( reader );
	assertEquals( versionStr(reader), versionStr(readerList.front()) );

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
	MyMemberId("me");

	TestableRamDataStore dataStore;
	dataStore.store("foo", "readme");

	StringBackedByteStream stream;
	IDataStoreReader::ptr reader = dataStore.read("foo", "1|me:1");
	assertTrue( reader );

	IDataStoreWriter::ptr writer = dataStore.write("foo");
	assertTrue( writer->write("0123456789", 10) );
	assertTrue( writer->commit() );

	vector<IDataStoreReader::ptr> lateReaderList = dataStore.read("foo");
	assertEquals( 1, lateReaderList.size() );
	assertEquals( "me:2", versionStr(lateReaderList.front()) );

	IDataStoreReader::ptr lateReader = dataStore.read("foo", "1|me:2");
	assertTrue( lateReader );
	assertEquals( versionStr(lateReader), versionStr(lateReaderList.front()) );

	assertEquals( 10, lateReader->read(stream) );
	assertEquals( "0123456789", stream._buffer );

	// now, the original reader
	assertEquals( 6, reader->read(stream) );
	assertEquals( "readme", stream._buffer );
}

TEST_CASE( "RamDataStoreTest/testReport", "[unit]" )
{
	TestableRamDataStore dataStore;
	dataStore.store("foo", "bytes");
	dataStore.store("foobar", "bytes");
	dataStore.store("bar", "bytes");
	assertEquals( 3, dataStore._store.size() );

	StringByteStream stream;
	dataStore.report(stream);

	// TODO: have a proper ByteStream mock...
	assertEquals( "(foo)=>5|1|me:1\n"
				  "(bar)=>5|1|me:1\n"
				  "(foobar)=>5|1|me:1", stream.writeBuffer() );
}

TEST_CASE( "RamDataStoreTest/testReport.Exclude", "[unit]" )
{
	TestableRamDataStore dataStore;
	dataStore.store("foo", "bytes");
	dataStore.store("foobar", "bytes");
	dataStore.store("bar", "bytes");

	StringByteStream stream;
	dataStore.report(stream, "foo");

	assertEquals( "(bar)=>5|1|me:1", stream.writeBuffer() );
}
