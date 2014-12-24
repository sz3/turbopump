/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "FileStore.h"

#include "readstream.h"
#include "writestream.h"
#include "common/MyMemberId.h"
#include "serialize/StringUtil.h"
#include "socket/NullByteStream.h"
#include "time/stopwatch.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <vector>
using std::string;
using std::vector;
using turbo::stopwatch;

namespace {
	string _test_dir = "/run/shm/turbo_fs_test";

	class DirectoryCleaner
	{
	public:
		~DirectoryCleaner()
		{
			boost::filesystem::remove_all(_test_dir);
		}

	protected:
	};
}

TEST_CASE( "StoreBenchmarkTest/testFileStore", "[unit]" )
{
	MyMemberId("increment");
	DirectoryCleaner cleaner;

	int trials = 50;
	long long writeMicros = 0;
	long long overwriteMicros = 0;
	long long readMicros = 0;

	FileStore store(_test_dir);
	for (int i = 0; i < trials; ++i)
	{
		stopwatch t;
		writestream stream = store.write( StringUtil::str(i) );
		stream.write("0123456789", 10);
		stream.commit(true);

		long long micros = t.micros();
		writeMicros += micros;
		std::cout << " fresh " << i << ": " << micros << "us" << std::endl;
	}

	for (int i = 0; i < trials; ++i)
	{
		stopwatch t;
		writestream stream = store.write( StringUtil::str(i) );
		stream.write("0123456789", 10);
		stream.commit(true);

		long long micros = t.micros();
		overwriteMicros += micros;
		std::cout << " overwrite " << i << ": " << micros << "us" << std::endl;
	}

	NullByteStream output;
	for (int i = 0; i < trials; ++i)
	{
		stopwatch t;
		readstream reader = store.read( StringUtil::str(i) );
		int bytes = reader.stream(output);

		long long micros = t.micros();
		readMicros += micros;
		std::cout << " read " << i << ": " << micros << "us for " << bytes << " bytes" << std::endl;
	}

	std::cout << " ***** average write: " << (writeMicros*1.0/trials) << "us" << std::endl;
	std::cout << " ***** average overwrite: " << (overwriteMicros*1.0/trials) << "us" << std::endl;
	std::cout << " ***** average read: " << (readMicros*1.0/trials) << "us" << std::endl;
}
