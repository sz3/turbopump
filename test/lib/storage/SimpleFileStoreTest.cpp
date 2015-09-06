/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SimpleFileStore.h"

#include "FileWriter.h"
#include "readstream.h"
#include "writestream.h"
#include "common/WallClock.h"
#include "file/File.h"
#include "serialize/str.h"
#include "serialize/str_join.h"

#include <boost/filesystem.hpp>
#include <map>
#include <string>
#include <vector>
using std::string;
using std::vector;
using namespace turbo::str;

namespace {
	string _test_dir = "turbo_fs_test";
	unsigned _magic_time = 0x12345678;
	string _magic_version = "1,sec.U5Oo4W00000=";

	class DirectoryCleaner
	{
	public:
		~DirectoryCleaner()
		{
			boost::filesystem::remove_all(_test_dir);
		}

	protected:
	};

	class TestableFileStore : public SimpleFileStore
	{
	public:
		using SimpleFileStore::SimpleFileStore;

		using SimpleFileStore::filepath;
		using SimpleFileStore::mergedVersion;
		using SimpleFileStore::onWriteComplete;
	};

	unsigned long long write_file(SimpleFileStore& store, const std::string& name, const std::string& contents, const std::string& version="")
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

TEST_CASE( "SimpleFileStoreTest/testPaths", "[unit]" )
{
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);

	FilePaths paths = store.filepath("myfile");
	assertEquals( "turbo_fs_test/myfile", paths.current() );
	assertEquals( "turbo_fs_test/.deleted.myfile", paths.deleted() );
	assertEquals( "turbo_fs_test/.myfile~", paths.inprogress() );
	assertEquals( "turbo_fs_test/.myfile~~", paths.reserve() );

	paths = store.filepath("mydir/myfile");
	assertEquals( "turbo_fs_test/mydir/myfile", paths.current() );
	assertEquals( "turbo_fs_test/mydir/.deleted.myfile", paths.deleted() );
	assertEquals( "turbo_fs_test/mydir/.myfile~", paths.inprogress() );
	assertEquals( "turbo_fs_test/mydir/.myfile~~", paths.reserve() );
}

TEST_CASE( "SimpleFileStoreTest/testMergedVersion", "[unit]" )
{
	WallClock().unfreeze();
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);
	{
		VectorClock expected;
		assertEquals( expected.toString(), store.mergedVersion("myfile").toString() );
	}

	FilePaths paths = store.filepath("myfile");
	FileWriter writer(paths.current());
	writer.write("0123456789", 10);
	writer.close();

	{
		VectorClock expected;
		expected.increment("sec", WallClock().now());
		assertEquals( expected.toString(), store.mergedVersion("myfile").toString() );
	}

	File::rename(paths.current(), paths.inprogress());

	{
		VectorClock expected;
		assertEquals( expected.toString(), store.mergedVersion("myfile", false).toString() );
	}
	{
		VectorClock expected;
		expected.increment("sec", WallClock().now());
		assertEquals( expected.toString(), store.mergedVersion("myfile", true).toString() );
	}

}

TEST_CASE( "SimpleFileStoreTest/testWrite", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);
	writestream writer = store.write("foo/myfile");
	assertTrue( writer.good() );

	string bytes = "0123456789";
	assertEquals( 10, writer.write(bytes.data(), bytes.size()) );

	// won't be in the right place while the write is in progress.
	vector<string> versions = store.versions("foo/myfile");
	assertEquals( "", join(versions) );

	// but if we ask for in progress writes, we should see it.
	// unfortunately, the timestamp of an inprogress file write is its local modified time.
	versions = store.versions("foo/myfile", true);
	{
		VectorClock expected;
		expected.increment("sec", File::modified_time(store.filepath("foo/myfile").inprogress()));
		assertEquals( expected.toString(), join(versions) );
	}

	readstream reader = store.read("foo/myfile");
	assertFalse( reader.good() );

	// close/commit, and all is well.
	assertTrue( writer.commit(true) );

	versions = store.versions("foo/myfile");
	assertEquals( _magic_version, join(versions) );

	reader = store.read("foo/myfile");
	assertTrue( reader.good() );
	assertEquals( 10, reader.size() );

	reader = store.read("foo/myfile", _magic_version);
	assertTrue( reader.good() );
	assertEquals( 10, reader.size() );
	assertEquals( _magic_version, reader.version() );
}

TEST_CASE( "SimpleFileStoreTest/testWrite.Overwrite", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	writestream writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit(true);

	WallClock().freeze(_magic_time+1);
	writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("ooohhh.", 7);
	writer.commit(true);

	readstream reader = store.read("myfile");
	assertTrue( reader.good() );
	assertEquals( 7, reader.size() );
}

TEST_CASE( "SimpleFileStoreTest/testWrite.Delete", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	writestream writer = store.write("mydir/myfile");
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit(true);

	vector<string> versions = store.versions("mydir/myfile");
	assertEquals( 1, versions.size() );

	VectorClock vc;
	assertTrue( vc.fromString(versions.back()) );
	vc.markDeleted();

	writer = store.write("mydir/myfile", vc.toString());
	assertTrue( writer.good() );
	writer.commit(true);

	readstream reader = store.read("mydir/myfile");
	assertEquals( vc.toString(), reader.version() );
	assertTrue( reader.good() );
	assertEquals( 0, reader.size() );
}


TEST_CASE( "SimpleFileStoreTest/testWrite.OverwriteDelete", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	writestream writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit(true);

	vector<string> versions = store.versions("myfile");
	assertEquals( 1, versions.size() );

	WallClock().freeze(_magic_time+1);
	VectorClock vc;
	assertTrue( vc.fromString(versions.back()) );
	vc.markDeleted();

	writer = store.write("myfile", vc.toString());
	writer.commit(true);

	readstream reader = store.read("myfile");
	vc = VectorClock();
	vc.increment("sec");
	vc.markDeleted();
	assertEquals( vc.toString(), reader.version() );
	assertTrue( reader.good() );
	assertEquals( 0, reader.size() );

	WallClock().freeze(_magic_time+2);
	writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("ooohhh.", 7);
	writer.commit(true);

	reader = store.read("myfile");
	assertTrue( reader.good() );
	assertEquals( 7, reader.size() );
}

TEST_CASE( "SimpleFileStoreTest/testWrite.SpecificVersion", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	VectorClock vc;
	vc.increment("guid");

	writestream writer = store.write("myfile", vc.toString());
	assertTrue( writer.good() );
	writer.commit(true);

	readstream reader = store.read("myfile");
	assertTrue( reader.good() );
	assertEquals( 0, reader.size() );
}

TEST_CASE( "SimpleFileStoreTest/testWrite.RejectInprogress", "[unit]" )
{
	WallClock().unfreeze();
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	writestream writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit();

	writestream other = store.write("myfile");
	assertFalse( other.good() );
}

TEST_CASE( "SimpleFileStoreTest/testWrite.RejectExisting", "[unit]" )
{
	WallClock().unfreeze();
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	writestream writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit(true);

	writer = store.write("myfile");
	assertFalse( writer.good() );
}

TEST_CASE( "SimpleFileStoreTest/testRead", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;

	SimpleFileStore store(_test_dir);
	write_file(store, "myfile", "foobar");

	readstream reader = store.read("myfile");
	assertTrue( reader.good() );
	assertEquals( 6, reader.size() );

	reader = store.read("nofile");
	assertFalse( reader.good() );
}

TEST_CASE( "SimpleFileStoreTest/testRead.Version", "[unit]" )
{
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	VectorClock version;
	version.increment("sec", 0x1234567);
	write_file(store, "myfile", "foobar", version.toString());

	readstream reader = store.read("myfile", version.toString());
	assertTrue( reader.good() );
	assertEquals( 6, reader.size() );
	assertEquals( version.toString(), reader.version() );

	reader = store.read("nofile", version.toString());
	assertFalse( reader.good() );
}

TEST_CASE( "SimpleFileStoreTest/testRead.InProgress", "[unit]" )
{
	// doesn't work right now
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;

	SimpleFileStore store(_test_dir);

	writestream writer = store.write("myfile");
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit(false);

	readstream reader = store.read("myfile", "", true);
	assertFalse( reader.good() );
}

TEST_CASE( "SimpleFileStoreTest/testRead.InProgress.Version", "[unit]" )
{
	// right now, this can't work. The version isn't marked properly until the write is complete.
	DirectoryCleaner cleaner;
	SimpleFileStore store(_test_dir);

	VectorClock version;
	version.increment("sec", 0x1234567);
	writestream writer = store.write("myfile", version.toString());
	assertTrue( writer.good() );
	writer.write("01234", 5);
	writer.commit(false);

	readstream reader = store.read("myfile", version.toString(), true);
	assertFalse( reader.good() );
}

TEST_CASE( "SimpleFileStoreTest/testReadAll", "[unit]" )
{
	WallClock().freeze(_magic_time);
	DirectoryCleaner cleaner;

	SimpleFileStore store(_test_dir);
	write_file(store, "myfile", "foobar");

	std::vector<readstream> readers = store.readAll("myfile");
	assertEquals( 1, readers.size() );
	assertTrue( readers.front().good() );
	assertEquals( 6, readers.front().size() );

	readers = store.readAll("nofile");
	assertEquals( 0, readers.size() );
}

TEST_CASE( "SimpleFileStoreTest/testExists", "[unit]" )
{
	WallClock().unfreeze();
	DirectoryCleaner cleaner;

	SimpleFileStore store(_test_dir);

	VectorClock c1;
	c1.increment("sec", WallClock().now());
	write_file(store, "myfile", "foobar", c1.toString());

	assertTrue( store.exists("myfile", c1.toString()) );

	VectorClock c2;
	c2.increment("sec", WallClock().now() - 0x1000);
	assertTrue( store.exists("myfile", c2.toString()) );

	VectorClock c3;
	c3.increment("sec", WallClock().now() + 0x1000);
	assertFalse( store.exists("myfile", c3.toString()) );

	assertFalse( store.exists("nofile", c1.toString()) );
}

TEST_CASE( "SimpleFileStoreTest/testVersions", "[unit]" )
{
	WallClock().unfreeze();
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);
	FilePaths paths = store.filepath("myfile");

	std::vector<string> vs = store.versions("myfile");
	assertEquals( "", join(vs) );

	File::save(paths.current(), "012345");
	File::set_modified_time(paths.current(), 0x12345678);

	vs = store.versions("myfile");
	assertEquals( "1,sec.U5Oo4W00000=", join(vs) );

	File::save(paths.deleted(), "");
	File::set_modified_time(paths.deleted(), 0x12344321);

	vs = store.versions("myfile");
	assertEquals( "1,sec.U5Oo4W00000= 2,delete.8KCo4W00000=,sec.8KCo4W00000=", join(vs) );
	vs = store.versions("myfile", true);
	assertEquals( "1,sec.U5Oo4W00000= 2,delete.8KCo4W00000=,sec.8KCo4W00000=", join(vs) );

	File::save(paths.inprogress(), "aaaa");
	File::set_modified_time(paths.inprogress(), 0x12340000);

	vs = store.versions("myfile");
	assertEquals( "1,sec.U5Oo4W00000= 2,delete.8KCo4W00000=,sec.8KCo4W00000=", join(vs) );
	vs = store.versions("myfile", true);
	assertEquals( "1,sec.U5Oo4W00000= 2,delete.8KCo4W00000=,sec.8KCo4W00000= 1,sec.000o4W00000=", join(vs) );
}

TEST_CASE( "SimpleFileStoreTest/testRemove", "[unit]" )
{
	SimpleFileStore store(_test_dir);

	assertFalse( store.remove("testfile") );
	assertFalse( store.remove("foo/bar") );

	write_file(store, "testfile", "lmno");
	write_file(store, "foo/bar", "lmno");

	assertTrue( store.remove("foo/bar") );
	assertTrue( store.remove("testfile") );
}

TEST_CASE( "SimpleFileStoreTest/testOnWriteComplete", "[unit]" )
{
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);
	FilePaths paths(_test_dir, "myfile");

	// failure, no inprogress file
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time);
		assertFalse( store.onWriteComplete("myfile", md) );
		assertEquals( 0, md.digest );
	}

	// success, new file
	unsigned long long lastDigest = 0;
	File::save(paths.inprogress(), "aaaa");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time);
		assertTrue( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.current(), contents) );
		assertEquals( "aaaa", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertEquals( 0, md.digest );

		lastDigest = writestream::digest(md.version.toString(), 4);
	}

	// success, replacement
	File::save(paths.inprogress(), "foobar");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time+1);
		assertTrue( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.current(), contents) );
		assertEquals( "foobar", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertEquals( lastDigest, md.digest );

		lastDigest = writestream::digest(md.version.toString(), 6);
	}

	// failure, delete fails
	File::save(paths.inprogress(), "");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time);
		md.version.markDeleted(_magic_time);
		assertFalse( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.current(), contents) );
		assertEquals( "foobar", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertFalse( File::exists(paths.deleted()) );
		assertEquals( 0, md.digest );
	}

	// success, delete goes through
	File::save(paths.inprogress(), "");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time+1);
		md.version.markDeleted(_magic_time+1);
		assertTrue( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.deleted(), contents) );
		assertEquals( "", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertFalse( File::exists(paths.current()) );
		assertEquals( lastDigest, md.digest );

		lastDigest = writestream::digest(md.version.toString(), 0);
	}

	// failure, delete is higher version
	File::save(paths.inprogress(), "woo");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time);
		assertFalse( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.deleted(), contents) );
		assertEquals( "", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertFalse( File::exists(paths.current()) );
		assertEquals( 0, md.digest );
	}

	// failure, delete is same version
	File::save(paths.inprogress(), "woo");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time+1);
		assertFalse( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.deleted(), contents) );
		assertEquals( "", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertFalse( File::exists(paths.current()) );
		assertEquals( 0, md.digest );
	}

	// success, delete is lower version
	File::save(paths.inprogress(), "woo");
	{
		KeyMetadata md;
		md.version.increment("sec", _magic_time+2);
		assertTrue( store.onWriteComplete("myfile", md) );

		string contents;
		assertTrue( File::load(paths.current(), contents) );
		assertEquals( "woo", contents );

		assertFalse( File::exists(paths.inprogress()) );
		assertFalse( File::exists(paths.deleted()) );
		assertEquals( lastDigest, md.digest );
	}
}

TEST_CASE( "SimpleFileStoreTest/testEnumerate", "[unit]" )
{
	WallClock().freeze(0x12345678);
	DirectoryCleaner cleaner;

	SimpleFileStore store(_test_dir);

	std::map<string, unsigned long long> digests;
	digests["foo"] = write_file(store, "foo", "0123456789");
	digests["bar"] = write_file(store, "bar", "abcde");
	digests["woo/hoo"] = write_file(store, "woo/hoo", "lmno");

	VectorClock vc;
	vc.increment("sec");
	vc.markDeleted();
	digests["deleted"] = write_file(store, "deleted", "", vc.toString());

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
	std::vector<string> expected = {
		"bar => 5:" + _magic_version,
		"deleted => 0:" + vc.toString(),
		"foo => 10:" + _magic_version,
		"woo/hoo => 4:" + _magic_version
	};
	assertEquals( join(expected, '\n'), join(files, '\n') );
	assertEquals( join(digests), join(actualDigests) );
}
