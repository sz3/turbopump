/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "http/HttpResponse.h"
#include "serialize/StringUtil.h"
#include "time/Timer.h"
#include "time/WaitFor.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using std::string;

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
	Timer elapsed;
	char readBuff[100];
	for (int i = 0; i < 100; ++i)
	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		std::cout << "write " << i <<  " connection open at " << elapsed.micros() << "us" << std::endl;

		string name = StringUtil::str(i);
		string packet = cluster[1].headerForWrite(name, name.size()) + name;
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());
		std::cout << "write " << i <<  " bytes sent at " << elapsed.micros() << "us" << std::endl;

		size_t bytesRead = read(socket_fd, readBuff, 100);
		std::cout << "write " << i <<  " finished at " << elapsed.micros() << "us" << std::endl;

		close(socket_fd);
		std::cout << "write " << i <<  " connection close at " << elapsed.micros() << "us" << std::endl;

		assertStringContains( "200 Success", string(readBuff, bytesRead) );
		fileList.push_back("(" + name + ")=>" + StringUtil::str(name.size()) + "|1,1:1");
	}
	std::cout << "did 100 writes in " << elapsed.millis() << "ms" << std::endl;

	std::sort(fileList.begin(), fileList.end());
	string expected = StringUtil::join(fileList, '\n');
	string response;
	Timer t;
	waitFor(30, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});

	// check contents of each file?
}

TEST_CASE( "ReadWriteLoadTest/testBigWrite", "[integration]" )
{
	std::vector<string> timingData;

	TurboCluster cluster(2, "--clone");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	char readBuff[100];

	Timer elapsed;
	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		timingData.push_back("opened write socket at " + StringUtil::str(elapsed.micros()) + "us");

		string packet = TurboRunner::headerForWrite("bigfile", bufsize);
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = 48+(c>>10);
		bytesWrit = write(socket_fd, buffer, bufsize);
		timingData.push_back("wrote all bytes at " + StringUtil::str(elapsed.micros()) + "us");

		size_t bytesRead = read(socket_fd, readBuff, 100);
		close(socket_fd);
		timingData.push_back("finished write at " + StringUtil::str(elapsed.micros()) + "us");

		assertEquals(bufsize, bytesWrit);
		assertStringContains( "200 Success", string(readBuff, bytesRead) );
	}

	string expectedContents = string(buffer, bufsize);
	memset(buffer, 0, bufsize);
	HttpResponse resp;

	{
		int socket_fd = openStreamSocket(cluster[1].dataChannel());
		timingData.push_back("opened read socket at " + StringUtil::str(elapsed.micros()) + "us");

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

	timingData.push_back("finished read 1 at " + StringUtil::str(elapsed.micros()) + "us");
	string actualContents = resp.body();
	assertEquals( expectedContents, actualContents );


	actualContents = cluster[1].query("read", "name=bigfile");
	assertEquals( expectedContents, actualContents );

	timingData.push_back("finished read 2 at " + StringUtil::str(elapsed.micros()) + "us");
	//std::cout << "finished read 2 at " << elapsed.micros() << "us" << std::endl;


	Timer t;
	while (t.millis() < 10000)
	{
		actualContents = cluster[2].query("read", "name=bigfile");
		if (!actualContents.empty())
			break;
	}
	timingData.push_back("finished read 3 at " + StringUtil::str(elapsed.millis()) + "ms");
	//std::cout << "read 3 finished at " << elapsed.millis() << "ms" << std::endl;
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

		string name = StringUtil::str(i);
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
	Timer t;
	waitFor(30, results.size() + " != 90", [&]()
	{
		string response = cluster[2].local_list();
		results = StringUtil::split(response, '\n');
		return results.size() == 90;
	});

	std::vector<string> badResults;
	for (std::vector<string>::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		if (it->find("65536") == string::npos)
			badResults.push_back(*it);
	}
	assertEquals( "", StringUtil::join(badResults, '\n') );

	// if we got this far, we would then check the contents of each file
}
