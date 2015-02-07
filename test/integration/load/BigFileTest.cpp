/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "storage/FileReader.h"

#include "serialize/str.h"
#include "socket/FileByteStream.h"
#include "time/stopwatch.h"
#include "time/wait_for.h"
#include <sys/socket.h>
#include <sys/un.h>
using std::string;
using turbo::str::str;
using turbo::stopwatch;

namespace {
	string exePath = string(TURBOPUMP_PROJECT_ROOT) + "/build/src/exe/turbopump/turbopump";

	int openStreamSocket(string where)
	{
		struct sockaddr_un address;
		memset(&address, 0, sizeof(struct sockaddr_un));
		address.sun_family = AF_UNIX;
		snprintf(address.sun_path, where.size()+1, where.c_str());

		int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
		assertTrue( socket_fd >= 0 );
		assertTrue( connect(socket_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == 0 );

		return socket_fd;
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

	string expected = "bigfile => " + str(originalSize) + "|1,1:1";
	assertEquals( expected, cluster[1].local_list() );

	string response;
	wait_for(8, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});
	wait_for(8, expected + " != " + response, [&]()
	{
		response = cluster[3].local_list();
		return expected == response;
	});
}

