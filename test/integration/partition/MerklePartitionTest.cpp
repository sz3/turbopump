/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "consistent_hashing/HashRing.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "serialize/StringUtil.h"
#include "time/WaitFor.h"

#include <exception>
using std::string;

namespace {
	string hashRingStr(string id)
	{
		return HashRing::hash(id) + ":" + id + " ";
	}
}

TEST_CASE( "MerklePartitionTest/testSync", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --no-write-chaining");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string response = cluster[1].query("membership");
	assertEquals( "1 127.0.0.1:9001\n"
				  "2 127.0.0.1:9002\n"
				  "3 127.0.0.1:9003\n"
				  "4 127.0.0.1:9004\n"
				  "5 127.0.0.1:9005\n"
				  "6 127.0.0.1:9006", response );

	// the hash ring order is 2, 6, 1, 5, 4, 3.
	// e.g. if the primary location is 2, the secondary is 6, the tertiary is 1...
	string expected = hashRingStr("2") + hashRingStr("6") + hashRingStr("1") + hashRingStr("5") + hashRingStr("4") + hashRingStr("3");
	response = cluster[1].query("ring");
	assertEquals(expected, response);

	// write each file to its primary location. Rely on healing to synchronize everything else.
	// setting n=3 here, so we get 3 "proper" copies of the file instead of 2. (also, if we decide to change the defaults at any point...)
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=" + num + " n=3|hello" + num + "' | nc -U " + cluster[i].dataChannel());
		assertEquals( "", response );
	}

	// wait for files to propagate
	// again, 2,6,1,5,4,3
	waitFor(60, response, [&]()
	{
		response = cluster[1].query("local_list");
		return "(1)=>7\n"
			   "(2)=>7\n"
			   "(6)=>7" == response;
	});

	waitFor(40, response, [&]()
	{
		response = cluster[2].query("local_list");
		return "(2)=>7\n"
			   "(3)=>7\n"
			   "(4)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[3].query("local_list");
		return "(3)=>7\n"
			   "(4)=>7\n"
			   "(5)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[4].query("local_list");
		return "(1)=>7\n"
			   "(4)=>7\n"
			   "(5)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[5].query("local_list");
		return "(1)=>7\n"
			   "(5)=>7\n"
			   "(6)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[6].query("local_list");
		return "(2)=>7\n"
			   "(3)=>7\n"
			   "(6)=>7" == response;
	});
}

TEST_CASE( "MerklePartitionTest/testRedistribute", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --no-write-chaining");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string response = cluster[1].query("membership");
	assertEquals( "1 127.0.0.1:9001\n"
				  "2 127.0.0.1:9002\n"
				  "3 127.0.0.1:9003\n"
				  "4 127.0.0.1:9004\n"
				  "5 127.0.0.1:9005\n"
				  "6 127.0.0.1:9006", response );

	// the hash ring order is 2, 6, 1, 5, 4, 3.
	// e.g. if the primary location is 2, the secondary is 6, the tertiary is 1...
	string expected = hashRingStr("2") + hashRingStr("6") + hashRingStr("1") + hashRingStr("5") + hashRingStr("4") + hashRingStr("3");
	response = cluster[1].query("ring");
	assertEquals(expected, response);

	// write ONLY to worker 1.
	// setting n=3 here, so we get 3 "proper" copies of the file instead of 2. (also, if we decide to change the defaults at any point...)
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=" + num + " n=3|hello" + num + "' | nc -U " + cluster[1].dataChannel());
		assertEquals( "", response );
	}

	// wait for files to propagate
	// again, 2,6,1,5,4,3
	// runner1 should drop the files he's not responsible for
	waitFor(60, response, [&]()
	{
		response = cluster[1].query("local_list");
		return "(1)=>7\n"
			   "(2)=>7\n"
			   "(6)=>7" == response;
	});

	waitFor(40, response, [&]()
	{
		response = cluster[2].query("local_list");
		return "(2)=>7\n"
			   "(3)=>7\n"
			   "(4)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[3].query("local_list");
		return "(3)=>7\n"
			   "(4)=>7\n"
			   "(5)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[4].query("local_list");
		return "(1)=>7\n"
			   "(4)=>7\n"
			   "(5)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[5].query("local_list");
		return "(1)=>7\n"
			   "(5)=>7\n"
			   "(6)=>7" == response;
	});

	waitFor(20, response, [&]()
	{
		response = cluster[6].query("local_list");
		return "(2)=>7\n"
			   "(3)=>7\n"
			   "(6)=>7" == response;
	});
}
