/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "storage/FileReader.h"
#include "storage/FileWriter.h"

#include "serialize/format.h"
#include "serialize/str.h"
#include "socket/FileByteStream.h"
#include "socket/local_stream_socket.h"
#include "socket/socket_address.h"
#include "time/stopwatch.h"
#include "time/wait_for.h"

#include <iostream>
using std::string;
using namespace turbo;

namespace {
	string exePath = string(TURBOPUMP_PROJECT_ROOT) + "/build/src/exe/turbopumpd/turbopumpd";

	int openStreamSocket(string where)
	{
		local_stream_socket sock;
		assertTrue( sock.connect(socket_address(where)) );
		return sock.handle();
	}
}

TEST_CASE( "BigFileTest/testDiskWrite", "disk" )
{
	// in theory and in practice, the overhead to read from disk will be slow enough to trip some race conditions (buffers not being ready, etc)
	// hope is to catch them.
	TurboCluster cluster(3, "--no-active-sync");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	FileReader reader(exePath);
	assertTrue( reader.good() );
	unsigned long long originalSize = reader.size();

	stopwatch elapsed;
	{
		FileByteStream out(openStreamSocket(cluster[1].dataChannel()));

		string packet = TurboRunner::headerForWrite("bigfile", originalSize);
		size_t bytesWrit = out.write(packet.data(), packet.size());

		while (reader.stream(out) > 0);

		char readBuff[100];
		size_t bytesRead = out.read(readBuff, 100);
		out.close();

		assertStringContains( "200 Success", string(readBuff, bytesRead) );
	}

	string expected = "bigfile => " + str::str(originalSize) + ":1,1.[^\\. ]+";
	assertMatch( expected, cluster[1].local_list() );

	string response = wait_for_match(8, expected, [&]()
	{
		return cluster[2].local_list();
	});
	wait_for_equal(8, response, [&]()
	{
		return cluster[3].local_list();
	});
}

TEST_CASE( "BigFileTest/testFinishPartialWrite", "[integration]" )
{
	TurboCluster cluster(2);
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	// shut down second worker
	cluster[2].stop(10, true);

	unsigned ONE_GB = 100000000;
	{
		FileWriter writer("hugefile");
		for (unsigned i = 0; i < (ONE_GB / 20); ++i)
			writer.write("0123456789abcdefghij", 20);
	}

	// write files to running worker
	stopwatch elapsed;
	{
		string response = cluster[1].post("copy", "name=hugefile&path=../hugefile");
		assertEquals( "200", response );
	}

	// inexact science. Start up second worker, hope it begins transfer, bounce it
	cluster[2].start();
	assertMsg( cluster[2].waitForRunning(), cluster.lastError() );
	cluster[2].stop(0, true);
	cluster[2].start();
	assertMsg( cluster[2].waitForRunning(), cluster.lastError() );

	// wait for files to propagate
	string expected = fmt::format(
		"hugefile => {0}:1,1.{1}"
		, ONE_GB
		, "[^\\. ]+"
	);
	string response = wait_for_match(5, expected, [&]()
	{
		string res = cluster[1].local_list();
		return res;
	});

	wait_for_equal(30, response, [&]()
	{
		return cluster[2].local_list();
	});
}


