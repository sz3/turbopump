/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "system/popen.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "http/HttpResponse.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include "time/stopwatch.h"
#include "time/wait_for.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using std::string;
using turbo::stopwatch;
using turbo::wait_for_equal;
using turbo::wait_for_match;
using namespace turbo::str;

namespace {
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

TEST_CASE( "ReadWriteLoadTest/testSmallWrites", "[integration]" )
{
	TurboCluster cluster(2, "--clone");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	std::vector<string> fileList;
	stopwatch elapsed;
	char readBuff[100];
	for (int i = 0; i < 300; ++i)
	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		std::cout << "write " << i <<  " connection open at " << elapsed.micros() << "us" << std::endl;

		string name = str(i);
		string packet = cluster[1].headerForWrite(name, name.size()) + name;
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());
		std::cout << "write " << i <<  " bytes sent at " << elapsed.micros() << "us" << std::endl;

		size_t bytesRead = read(socket_fd, readBuff, 100);
		std::cout << "write " << i <<  " finished at " << elapsed.micros() << "us" << std::endl;

		close(socket_fd);
		std::cout << "write " << i <<  " connection close at " << elapsed.micros() << "us" << std::endl;

		assertStringContains( "200 Success", string(readBuff, bytesRead) );
		fileList.push_back(name + " => " + str(name.size()) + ":1,1\\.[^\\. ]+");
	}
	std::cout << "did 300 writes in " << elapsed.millis() << "ms" << std::endl;

	std::sort(fileList.begin(), fileList.end());
	string expected = join(fileList, '\n');
	wait_for_match(30, expected, [&]()
	{
		return cluster[2].local_list();
	});

	// check contents of each file?
}


TEST_CASE( "ReadWriteLoadTest/testBigWrite.Solo", "[integration]" )
{
	std::cout << std::endl << "*** testBigWrite.Solo ***" << std::endl;
	std::vector<string> timingData;

	TurboCluster cluster(1, "--clone");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	char readBuff[100];

	stopwatch elapsed;
	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		timingData.push_back("opened write socket at " + str(elapsed.micros()) + "us");

		string packet = TurboRunner::headerForWrite("bigfile", bufsize);
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = 48+(c>>10);
		bytesWrit = write(socket_fd, buffer, bufsize);
		timingData.push_back("wrote all bytes at " + str(elapsed.micros()) + "us");

		size_t bytesRead = read(socket_fd, readBuff, 100);
		close(socket_fd);
		timingData.push_back("write returned at " + str(elapsed.micros()) + "us");

		assertEquals(bufsize, bytesWrit);
		assertStringContains( "200 Success", string(readBuff, bytesRead) );
	}

	string expectedContents = string(buffer, bufsize);
	string actualContents = cluster[1].query("read", "name=bigfile");
	assertEquals( expectedContents, actualContents );
	timingData.push_back("finished read at " + str(elapsed.micros()) + "us");

	for (std::vector<string>::const_iterator it = timingData.begin(); it != timingData.end(); ++it)
		std::cout << *it << std::endl;
}

TEST_CASE( "ReadWriteLoadTest/testBigWrite.Duo", "[integration]" )
{
	std::cout << std::endl << "*** testBigWrite.Duo ***" << std::endl;
	std::vector<string> timingData;

	TurboCluster cluster(2, "--clone");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	char readBuff[100];

	stopwatch elapsed;
	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		timingData.push_back("opened write socket at " + str(elapsed.micros()) + "us");

		string packet = TurboRunner::headerForWrite("bigfile", bufsize);
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = 48+(c>>10);
		bytesWrit = write(socket_fd, buffer, bufsize);
		timingData.push_back("wrote all bytes at " + str(elapsed.micros()) + "us");

		size_t bytesRead = read(socket_fd, readBuff, 100);
		close(socket_fd);
		timingData.push_back("write returned at " + str(elapsed.micros()) + "us");

		assertEquals(bufsize, bytesWrit);
		assertStringContains( "200 Success", string(readBuff, bytesRead) );
	}

	string expectedContents = string(buffer, bufsize);
	memset(buffer, 0, bufsize);
	HttpResponse resp;

	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		timingData.push_back("opened read socket at " + str(elapsed.micros()) + "us");

		string packet = TurboRunner::headerForRead("bigfile");
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());
		assertEquals( packet.size(), bytesWrit );

		size_t totalBytes = 0;
		int bytesRead = 0;
		while (totalBytes < 65631 && (bytesRead = read(socket_fd, buffer, bufsize)) > 0)
		{
			totalBytes += bytesRead;
			resp.parse(buffer, bytesRead);
		}
		close(socket_fd);
		assertEquals(65631, totalBytes);
	}

	timingData.push_back("finished read 1 at " + str(elapsed.micros()) + "us");
	string actualContents = resp.body();
	assertEquals( expectedContents, actualContents );

	actualContents = cluster[1].query("read", "name=bigfile");
	assertEquals( expectedContents, actualContents );
	timingData.push_back("finished read 2 at " + str(elapsed.micros()) + "us");

	stopwatch t;
	while (t.millis() < 10000)
	{
		actualContents = cluster[2].query("read", "name=bigfile");
		if (!actualContents.empty())
			break;
	}
	timingData.push_back("finished read 3 at " + str(elapsed.millis()) + "ms");
	assertEquals( expectedContents, actualContents );

	for (std::vector<string>::const_iterator it = timingData.begin(); it != timingData.end(); ++it)
		std::cout << *it << std::endl;
}

TEST_CASE( "ReadWriteLoadTest/testManyBigWrites", "[integration]" )
{
	TurboCluster cluster(2, "--clone");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	for (int i = 32; i < 122; ++i)
	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());

		string name = str(i);
		string packet = cluster[1].headerForWrite(name, bufsize);
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = i;
		bytesWrit = write(socket_fd, buffer, bufsize);

		size_t bytesRead = read(socket_fd, buffer, 100);
		close(socket_fd);

		assertEquals(bufsize, bytesWrit);
		assertStringContains( "200 Success", string(buffer, bytesRead) );
	}

	std::vector<string> results;
	stopwatch t;
	unsigned expected = 90;
	wait_for_equal(30, expected, [&]()
	{
		string response = cluster[2].local_list();
		results = split(response, '\n');
		return results.size();
	});

	std::vector<string> badResults;
	for (std::vector<string>::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		if (it->find("65536") == string::npos)
			badResults.push_back(*it);
	}
	assertEquals( "", join(badResults, '\n') );

	// if we got this far, we would then check the contents of each file
}
