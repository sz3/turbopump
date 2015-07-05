/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "storage/FileReader.h"

#include "serialize/str.h"
#include "socket/FileByteStream.h"
#include "socket/local_stream_socket.h"
#include "socket/socket_address.h"
#include "time/stopwatch.h"
#include "time/wait_for.h"
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

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	char readBuff[100];

	stopwatch elapsed;
	{
		FileByteStream out(openStreamSocket(cluster[1].dataChannel()));

		string packet = TurboRunner::headerForWrite("bigfile", originalSize);
		size_t bytesWrit = out.write(packet.data(), packet.size());

		while (reader.stream(out) > 0);

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

