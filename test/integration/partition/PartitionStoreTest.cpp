/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "hashing/Hash.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "serialize/str.h"
#include "serialize/str_join.h"
#include <vector>
using std::string;
using std::vector;
using turbo::str::str;

namespace {
	string hashStr(string id)
	{
		return Hash(id).base64() + "=" + id;
	}
}

TEST_CASE( "PartitionStoreTest/testFilePlacement", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --no-active-sync");
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
	vector<string> expected{hashStr("2"), hashStr("6"), hashStr("1"), hashStr("5"), hashStr("4"), hashStr("3")};
	response = cluster[1].query("ring");
	assertEquals(turbo::str::join(expected), response);

	// setting copies=3 here, so we get 3 "proper" copies of the file instead of 2.
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = str(i);
		response = cluster[1].write(num, "hello!"+num, "copies=3");
		assertEquals( "200", response );
	}

	// wait for files to propagate
	// TODO: don't
	CommandLine::run("sleep 0.5");

	// again, 2,6,1,5,4,3
	response = cluster[2].local_list();
	assertEquals( "2 => 7|1,1:1\n"
				  "3 => 7|1,1:1\n"
				  "4 => 7|1,1:1", response );

	response = cluster[3].local_list();
	assertEquals( "3 => 7|1,1:1\n"
				  "4 => 7|1,1:1\n"
				  "5 => 7|1,1:1", response );

	response = cluster[4].local_list();
	assertEquals( "1 => 7|1,1:1\n"
				  "4 => 7|1,1:1\n"
				  "5 => 7|1,1:1", response );

	response = cluster[5].local_list();
	assertEquals( "1 => 7|1,1:1\n"
				  "5 => 7|1,1:1\n"
				  "6 => 7|1,1:1", response );

	response = cluster[6].local_list();
	assertEquals( "2 => 7|1,1:1\n"
				  "3 => 7|1,1:1\n"
				  "6 => 7|1,1:1", response );

	// runner1 should drop the files he's not responsible for
	response = cluster[1].local_list();
	assertEquals( "1 => 7|1,1:1\n"
				  "2 => 7|1,1:1\n"
				  "6 => 7|1,1:1", response );
}

TEST_CASE( "PartitionStoreTest/testVariableReplication", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --sync-interval 250");
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
	vector<string> expected{hashStr("2"), hashStr("6"), hashStr("1"), hashStr("5"), hashStr("4"), hashStr("3")};
	response = cluster[1].query("ring");
	assertEquals(turbo::str::join(expected), response);

	// write 5 files, all with different replication...
	for (unsigned i = 1; i <= 5; ++i)
	{
		string num = str(i);
		response = cluster[1].write(num, "hello!"+num, "copies="+num);
		assertEquals( "200", response );
	}

	// wait for files to propagate
	// TODO: don't
	CommandLine::run("sleep 0.5");

	// again, 2,6,1,5,4,3
	// that is:
	// (1) => 1
	// (2) => 2,6
	// (3) => 3,2,6
	// (4) => 4,3,2,6
	// (5) => 5,4,3,2,6

	response = cluster[2].local_list();
	assertEquals( "2 => 7|1,1:1\n"
				  "3 => 7|1,1:1\n"
				  "4 => 7|1,1:1\n"
				  "5 => 7|1,1:1", response );

	response = cluster[3].local_list();
	assertEquals( "3 => 7|1,1:1\n"
				  "4 => 7|1,1:1\n"
				  "5 => 7|1,1:1", response );

	response = cluster[4].local_list();
	assertEquals( "4 => 7|1,1:1\n"
				  "5 => 7|1,1:1", response );

	response = cluster[5].local_list();
	assertEquals( "5 => 7|1,1:1", response );

	response = cluster[6].local_list();
	assertEquals( "2 => 7|1,1:1\n"
				  "3 => 7|1,1:1\n"
				  "4 => 7|1,1:1\n"
				  "5 => 7|1,1:1", response );

	// runner1 should drop the files he's not responsible for
	response = cluster[1].local_list();
	assertEquals( "1 => 7|1,1:1", response );
}
