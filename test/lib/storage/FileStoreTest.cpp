/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "FileStore.h"

#include "readstream.h"
#include "writestream.h"
#include "common/MyMemberId.h"
#include "file/File.h"
#include "serialize/str_join.h"

#include <boost/filesystem.hpp>
#include <memory>
#include <string>
#include <vector>
using std::shared_ptr;
using std::string;
using std::vector;

namespace {
	string _test_dir = "/tmp/turbo_fs_test";

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

		using FileStore::bestVersion;
		using FileStore::dirpath;
		using FileStore::filepath;
	};
}

TEST_CASE( "FileStoreTest/testPaths", "[unit]" )
{
	DirectoryCleaner cleaner;

	TestableFileStore store(_test_dir);
	assertEquals( "/tmp/turbo_fs_test/rho2ybhjCAs2eRlIaSZVWroidvn5DwZ2/myfile",  store.dirpath("myfile") );
	assertEquals( "/tmp/turbo_fs_test/rho2ybhjCAs2eRlIaSZVWroidvn5DwZ2/myfile/v1",  store.filepath("myfile", "v1") );
}

TEST_CASE( "FileStoreTest/testWrite", "[unit]" )
{
	MyMemberId("1");
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);
	writestream writer = store.write("myfile");
	assertTrue( writer.good() );

	string bytes = "0123456789";
	assertEquals( 10, writer.write(bytes.data(), bytes.size()) );

	// won't be in the right place while the write is in progress.
	vector<string> versions = store.versions("myfile");
	assertEquals( "", turbo::str::join(versions) );

	// but if we ask for in progress writes, we should see it.
	versions = store.versions("myfile", true);
	assertEquals( "1,1:1", turbo::str::join(versions) );

	readstream reader = store.read("myfile", "1,1:1");
	assertFalse( reader.good() );

	// close/commit, and all is well.
	assertTrue( writer.close() );

	versions = store.versions("myfile");
	assertEquals( "1,1:1", turbo::str::join(versions) );

	reader = store.read("myfile", "1,1:1");
	assertTrue( reader.good() );
	assertEquals( 10, reader.size() );
}

TEST_CASE( "FileStoreTest/testReadNewest", "[unit]" )
{
	MyMemberId("1");
	DirectoryCleaner cleaner;

	FileStore store(_test_dir);

	{
		writestream writer = store.write("myfile");
		assertTrue( writer.good() );

		string bytes = "0123456789";
		assertEquals( 10, writer.write(bytes.data(), bytes.size()) );
		assertTrue( writer.close() );
	}
	{
		readstream reader = store.read("myfile");
		assertTrue( reader.good() );
		assertEquals( "1,1:1", reader.version() );
		assertEquals( 10, reader.size() );
	}

	{
		writestream writer = store.write("myfile");
		assertTrue( writer.good() );

		string bytes = "abcdef";
		assertEquals( 6, writer.write(bytes.data(), bytes.size()) );
		assertTrue( writer.close() );
	}
	{
		readstream reader = store.read("myfile");
		assertTrue( reader );
		assertEquals( "1,1:2", reader.version() );
		assertEquals( 6, reader.size() );
	}
}

