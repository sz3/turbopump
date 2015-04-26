/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "FileStore.h"

#include "readstream.h"
#include "writestream.h"
#include "common/MyMemberId.h"
#include "common/WallClock.h"
#include "file/File.h"
#include "serialize/str.h"

#include <boost/filesystem.hpp>
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>
using std::shared_ptr;
using std::string;
using std::vector;
using namespace turbo::str;

namespace {
	string _test_dir = "turbo_fs_test";

	class DirectoryCleaner
	{
	public:
		~DirectoryCleaner()
		{
			boost::filesystem::remove_all(_test_dir);
		}

	protected:
	};

	class TestableFileStore : public FileStore
	{
	public:
		using FileStore::FileStore;

		using FileStore::mergedVersion;
		using FileStore::dirpath;
		using FileStore::filepath;
	};

	unsigned long long write_file(FileStore& store, const std::string& name, const std::string& contents, const std::string& version="")
	{
		writestream writer = store.write(name, version);
		assert( writer.good() );
		writer.write(contents.data(), contents.size());
		return writer.commit(true).digest();
	}

	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}
#include "serialize/str_join.h"

TEST_CASE( "FileStoreTest/testPaths", "[unit]" )
{
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);
	assertEquals( "turbo_fs_test/myfile",  store.dirpath("myfile") );
	assertEquals( "turbo_fs_test/myfile/v1",  store.filepath("myfile", "v1") );
}

TEST_CASE( "FileStoreTest/testWrite", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);
	writestream writer = store.write("myfile");
	assertTrue( writer.good() );

	string bytes = "0123456789";
	assertEquals( 10, writer.write(bytes.data(), bytes.size()) );

	// won't be in the right place while the write is in progress.
	vector<string> versions = store.versions("myfile");
	assertEquals( "", join(versions) );

	// but if we ask for in progress writes, we should see it.
	versions = store.versions("myfile", true);
	assertEquals( "1,increment.UNIXSECONDS=", join(versions) );

	readstream reader = store.read("myfile", "1,increment.UNIXSECONDS=");
	assertFalse( reader.good() );

	// close/commit, and all is well.
	assertTrue( writer.commit(true) );

	versions = store.versions("myfile");
	assertEquals( "1,increment.UNIXSECONDS=", join(versions) );

	reader = store.read("myfile", "1,increment.UNIXSECONDS=");
	assertTrue( reader.good() );
	assertEquals( 10, reader.size() );
}

TEST_CASE( "FileStoreTest/testWrite.RejectInprogress", "[unit]" )
{
	MyMemberId("increment");
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	VectorClock version;
	version.increment("jordy");
	version.increment("randall");
	writestream writer = store.write("myfile", version.toString());
	assertTrue( writer.good() );

	writestream other = store.write("myfile", version.toString());
	assertFalse( other.good() );
}

TEST_CASE( "FileStoreTest/testWrite.RejectExisting", "[unit]" )
{
	MyMemberId("increment");
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	VectorClock version;
	version.increment("jordy");
	version.increment("randall");
	{
		writestream writer = store.write("myfile", version.toString());
		assertTrue( writer.good() );
		writer.commit(true);
	}

	writestream writer = store.write("myfile", version.toString());
	assertFalse( writer.good() );
}

TEST_CASE( "FileStoreTest/testOverwrite", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	write_file(store, "myfile", "0123456789");
	write_file(store, "myfile", "abcdef");
	write_file(store, "myfile", "ha ha ha!");

	vector<string> versions = store.versions("myfile");
	assertEquals( "1,increment.UNIXSECONDS=.2", join(versions) );

	WallClock().freeze(WallClock::MAGIC_NUMBER+8);
	VectorClock vnew;
	vnew.increment("increment");
	write_file(store, "myfile", "one more write", vnew.toString());

	assertEquals( "1,increment.WNIXSECONDS=", join(store.versions("myfile")) );
}

TEST_CASE( "FileStoreTest/testOverwrite.TimeLapse", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	write_file(store, "myfile", "0123456789");

	vector<string> versions = store.versions("myfile");
	assertEquals( "1,increment.UNIXSECONDS=", join(versions) );

	WallClock().freeze(WallClock::MAGIC_NUMBER-8);
	write_file(store, "myfile", "time can't go backwards...");

	// so we just do increment the secondary count...
	assertEquals( "1,increment.UNIXSECONDS=.1", join(store.versions("myfile")) );

	VectorClock vnew;
	vnew.fromString(versions.front());
	vnew.increment("increment");
}

TEST_CASE( "FileStoreTest/testUnderwrite", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	write_file(store, "myfile", "0123456789");

	vector<string> versions = store.versions("myfile");
	assertEquals( "1,increment.UNIXSECONDS=", join(versions) );

	WallClock().freeze(WallClock::MAGIC_NUMBER-8);
	VectorClock vnew;
	vnew.increment("increment");
	write_file(store, "myfile", "ignore me :(", vnew.toString());

	assertEquals( "1,increment.UNIXSECONDS=", join(store.versions("myfile")) );

}

TEST_CASE( "FileStoreTest/testReadNewest", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	write_file(store, "myfile", "0123456789");
	{
		readstream reader = store.read("myfile");
		assertTrue( reader.good() );
		assertEquals( "1,increment.UNIXSECONDS=", reader.version() );
		assertEquals( 10, reader.size() );
	}

	write_file(store, "myfile", "abcdef");
	{
		readstream reader = store.read("myfile");
		assertTrue( reader );
		assertEquals( "1,increment.UNIXSECONDS=.1", reader.version() );
		assertEquals( 6, reader.size() );
	}
}

TEST_CASE( "FileStoreTest/testReadCantDecide", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	VectorClock v1;
	v1.increment("foo");
	write_file(store, "myfile", "0123456789", v1.toString());

	VectorClock v2;
	v2.increment("bar");
	write_file(store, "myfile", "abcdef", v2.toString());

	readstream reader = store.read("myfile");
	assertFalse( reader.good() );

	// fix the conflict
	write_file(store, "myfile", "fix!");

	reader = store.read("myfile");
	assertTrue( reader );
	assertEquals( "3,increment.UNIXSECONDS=,foo.UNIXSECONDS=,bar.UNIXSECONDS=", reader.version() );
	assertEquals( 4, reader.size() );
}

TEST_CASE( "FileStoreTest/testReadInprogress", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	writestream writer = store.write("myfile");
	assertTrue( writer.good() );

	string bytes = "0123456789";
	assertEquals( 10, writer.write(bytes.data(), bytes.size()) );

	// default, only read finished versions
	readstream reader = store.read("myfile");
	assertFalse( reader );

	// but how about an inprogress one?
	reader = store.read("myfile", "", true);
	assertTrue( reader );
	assertEquals( "1,increment.UNIXSECONDS=", reader.version() );
	assertEquals( 10, reader.size() );
}

TEST_CASE( "FileStoreTest/testReadAll", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	VectorClock v1;
	v1.increment("foo");
	write_file(store, "myfile", "0123456789", v1.toString());
	{
		readstream reader = store.read("myfile");
		assertTrue( reader.good() );
		assertEquals( "1,foo.UNIXSECONDS=", reader.version() );
		assertEquals( 10, reader.size() );
	}

	VectorClock v2;
	v2.increment("bar");
	write_file(store, "myfile", "abcdef", v2.toString());
	{
		readstream reader = store.read("myfile", v2.toString());
		assertTrue( reader );
		assertEquals( "1,bar.UNIXSECONDS=", reader.version() );
		assertEquals( 6, reader.size() );
	}

	std::vector<readstream> readers = store.readAll("myfile");
	assertEquals( 2, readers.size() );

	assertTrue( readers[0] );
	assertEquals( "1,foo.UNIXSECONDS=", readers[0].version() );
	assertEquals( 10, readers[0].size() );

	assertTrue( readers[1] );
	assertEquals( "1,bar.UNIXSECONDS=", readers[1].version() );
	assertEquals( 6, readers[1].size() );
}

TEST_CASE( "FileStoreTest/testReadWriteVariantCopies", "[unit]" )
{
	MyMemberId("increment");
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	for (int i = 0; i < 15; ++i)
	{
		writestream writer = store.write("myfile", "", i);
		writer.write("foo", 3);
		writer.commit(true);

		readstream reader = store.read("myfile");
		assertEquals(i, reader.mirrors());
	}
}

TEST_CASE( "FileStoreTest/testExists", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	VectorClock version;
	version.increment("woop");
	assertFalse( store.exists("foo", version.toString()) );
	write_file(store, "foo", "0123456789", version.toString());
	assertTrue( store.exists("foo", version.toString()) );

	assertFalse( store.exists("woo/hoo", version.toString()) );
	write_file(store, "woo/hoo", "lmno", version.toString());
	assertTrue( store.exists("woo/hoo", version.toString()) );
}

TEST_CASE( "FileStoreTest/testEnumerate", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);
	std::map<string, unsigned long long> digests;

	digests["foo"] = write_file(store, "foo", "0123456789");
	digests["bar"] = write_file(store, "bar", "abcde");
	digests["woo/hoo"] = write_file(store, "woo/hoo", "lmno");

	std::vector<string> files;
	std::map<string, unsigned long long> actualDigests;
	auto fun = [&files, &actualDigests] (const std::string& name, const KeyMetadata& md, const std::string& summary)
	{
		files.push_back(name + " => " + summary);
		actualDigests[name] = md.digest;
		return true;
	};
	store.enumerate(fun, 100);

	std::sort(files.begin(), files.end());
	assertEquals( "bar => 5:1,increment.UNIXSECONDS=\n"
				  "foo => 10:1,increment.UNIXSECONDS=\n"
				  "woo/hoo => 4:1,increment.UNIXSECONDS=", join(files, '\n') );
	assertEquals( join(digests), join(actualDigests) );
}

TEST_CASE( "FileStoreTest/testEnumerate.Detail", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);
	readstream reader;
	{
		writestream writer = store.write("foo/foo/foo", "", 5);
		writer.write("01234567", 8);
		reader = writer.commit(true);
	}

	string actualName;
	KeyMetadata actualMd;
	string actualSummary;
	auto fun = [&] (const std::string& name, const KeyMetadata& md, const std::string& summary)
	{
		actualName = name;
		actualMd = md;
		actualSummary = summary;
		return true;
	};
	store.enumerate(fun, 100);

	assertEquals( "foo/foo/foo", actualName );
	assertEquals( "8:1,increment.UNIXSECONDS=", actualSummary );
	assertEquals( reader.digest(), actualMd.digest );
	assertEquals( 5, actualMd.totalCopies );
}

TEST_CASE( "FileStoreTest/testEnumerate.Conflict", "[unit]" )
{
	MyMemberId("increment");
	WallClock().freeze(WallClock::MAGIC_NUMBER);
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);
	readstream readerOne;
	{
		VectorClock version;
		version.increment("one");
		writestream writer = store.write("foo", version.toString(), 0);
		writer.write("01234567", 8);
		readerOne = writer.commit(true);
	}

	readstream readerTwo;
	{
		VectorClock version;
		version.increment("two");
		writestream writer = store.write("foo", version.toString(), 0);
		writer.write("abcdef", 6);
		readerTwo = writer.commit(true);
	}

	string actualName;
	KeyMetadata actualMd;
	string actualSummary;
	auto fun = [&] (const std::string& name, const KeyMetadata& md, const std::string& summary)
	{
		actualName = name;
		actualMd = md;
		actualSummary = summary;
		return true;
	};
	store.enumerate(fun, 100);

	assertEquals( "foo", actualName );
	assertEquals( "6:1,two.UNIXSECONDS= 8:1,one.UNIXSECONDS=", join(sort(split(actualSummary, ' ', true)), ' ') );
	assertEquals( (readerOne.digest() xor readerTwo.digest()), actualMd.digest );
	assertEquals( 0, actualMd.totalCopies );
}
