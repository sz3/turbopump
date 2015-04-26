/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "hashing/Hash.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "cppformat/format.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include "time/wait_for.h"

#include <exception>
#include <vector>
using std::string;
using std::vector;
using namespace turbo;

namespace {
	string hashStr(string id)
	{
		return Hash(id).base64() + "=" + id;
	}
}

TEST_CASE( "MerklePartitionTest/testSync", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --no-write-chaining --sync-interval 250");
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
	vector<string> ring{hashStr("2"), hashStr("6"), hashStr("1"), hashStr("5"), hashStr("4"), hashStr("3")};
	response = cluster[1].query("ring");
	assertEquals(str::join(ring), response);

	// write each file to its primary location. Rely on healing to synchronize everything else.
	// setting copies=3 here, in case we decide to change the defaults at any point...
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = str::str(i);
		response = cluster[i].write(num, "hello!"+num, "copies=3");
		assertEquals( "200", response );
	}

	// wait for files to propagate
	// again, 2,6,1,5,4,3
	string expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"2 => 7:1,2.{0}\n"
		"6 => 7:1,6.{0}"
		, "([^. ]+)"
	);
	wait_for_match(60, expected, [&]()
	{
		return cluster[1].local_list();
	});

	expected = fmt::format(
		"2 => 7:1,2.{0}\n"
		"3 => 7:1,3.{0}\n"
		"4 => 7:1,4.{0}"
		, "([^. ]+)"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[2].local_list();
	});

	expected = fmt::format(
		"3 => 7:1,3.{0}\n"
		"4 => 7:1,4.{0}\n"
		"5 => 7:1,5.{0}"
		, "([^. ]+)"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[3].local_list();
	});

	expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"4 => 7:1,4.{0}\n"
		"5 => 7:1,5.{0}"
		, "([^. ]+)"
	);
	wait_for_match(20, expected, [&]()
	{
		return cluster[4].local_list();
	});

	expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"5 => 7:1,5.{0}\n"
		"6 => 7:1,6.{0}"
		, "([^. ]+)"
	);
	wait_for_match(20, expected, [&]()
	{
		return cluster[5].local_list();
	});

	expected = fmt::format(
		"2 => 7:1,2.{0}\n"
		"3 => 7:1,3.{0}\n"
		"6 => 7:1,6.{0}"
		, "([^. ]+)"
	);
	wait_for_match(20, expected, [&]()
	{
		return cluster[6].local_list();
	});
}

TEST_CASE( "MerklePartitionTest/testRedistribute", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --no-write-chaining --sync-interval 250");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	for (int i = 1; i <= 6; ++i)
	{
		string response = cluster[1].query("membership");
		assertEquals( "1 127.0.0.1:9001\n"
					  "2 127.0.0.1:9002\n"
					  "3 127.0.0.1:9003\n"
					  "4 127.0.0.1:9004\n"
					  "5 127.0.0.1:9005\n"
					  "6 127.0.0.1:9006", response );
	}

	// the hash ring order is 2, 6, 1, 5, 4, 3.
	// e.g. if the primary location is 2, the secondary is 6, the tertiary is 1...
	vector<string> ring{hashStr("2"), hashStr("6"), hashStr("1"), hashStr("5"), hashStr("4"), hashStr("3")};
	string response = cluster[1].query("ring");
	assertEquals(str::join(ring), response);

	// write ONLY to worker 1.
	// setting copies=3 here, in case we decide to change the defaults at any point...
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = str::str(i);
		response = cluster[1].write(num, "hello!"+num, "copies=3");
		assertEquals( "200", response );
	}

	// wait for files to propagate
	// again, 2,6,1,5,4,3
	// runner1 should drop the files he's not responsible for
	string expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"2 => 7:1,1.{0}\n"
		"6 => 7:1,1.{0}"
		, "([^. ]+)"
	);
	wait_for_match(70, expected, [&]()
	{
		return cluster[1].local_list();
	});

	expected = fmt::format(
		"2 => 7:1,1.{0}\n"
		"3 => 7:1,1.{0}\n"
		"4 => 7:1,1.{0}"
		, "([^. ]+)"
	);
	wait_for_match(60, expected, [&]()
	{
		return cluster[2].local_list();
	});

	expected = fmt::format(
		"3 => 7:1,1.{0}\n"
		"4 => 7:1,1.{0}\n"
		"5 => 7:1,1.{0}"
		, "([^. ]+)"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[3].local_list();
	});

	expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"4 => 7:1,1.{0}\n"
		"5 => 7:1,1.{0}"
		, "([^. ]+)"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[4].local_list();
	});

	expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"5 => 7:1,1.{0}\n"
		"6 => 7:1,1.{0}"
		, "([^. ]+)"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[5].local_list();
	});

	expected = fmt::format(
		"2 => 7:1,1.{0}\n"
		"3 => 7:1,1.{0}\n"
		"6 => 7:1,1.{0}"
		, "([^. ]+)"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[6].local_list();
	});
}

TEST_CASE( "MerklePartitionTest/testDualSync", "[integration-udp]" )
{
	TurboCluster cluster(3, "--udp --no-write-chaining --sync-interval 250");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	// write files to running worker
	string response = cluster[1].write("foo", "hello!", "copies=0");
	assertEquals( "200", response );

	response = cluster[1].write("bar", "bar?", "copies=3");
	assertEquals( "200", response );

	// wait for files to propagate
	string expected = fmt::format(
		"bar => 4:1,1.{0}\n"
		"foo => 6:1,1.{0}"
		, "[^. ]+"
	);
	expected = wait_for_match(5, expected, [&]()
	{
		return cluster[1].local_list();
	});

	wait_for_equal(60, expected, [&]()
	{
		return cluster[2].local_list();
	});

	wait_for_equal(60, expected, [&]()
	{
		return cluster[3].local_list();
	});
}


TEST_CASE( "MerklePartitionTest/testSyncMultipleTrees", "[integration-udp]" )
{
	TurboCluster cluster(6, "--udp --no-write-chaining --sync-interval 250");
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
	vector<string> ring{hashStr("2"), hashStr("6"), hashStr("1"), hashStr("5"), hashStr("4"), hashStr("3")};
	response = cluster[1].query("ring");
	assertEquals(str::join(ring), response);

	// write each file to its primary location. Rely on healing to synchronize everything else.
	// setting copies=i here, so we get a different merkle tree per write...
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = str::str(i);
		response = cluster[i].write(num, "hello!"+num, "copies="+num);
		assertEquals( "200", response );
	}

	// wait for files to propagate
	// again, 2,6,1,5,4,3
	string expected = fmt::format(
		"1 => 7:1,1.{0}\n"
		"6 => 7:1,6.{0}"
		, "[^. ]+"
	);
	wait_for_match(60, expected, [&]()
	{
		return cluster[1].local_list();
	});

	expected = fmt::format(
		"2 => 7:1,2.{0}\n"
		"3 => 7:1,3.{0}\n"
		"4 => 7:1,4.{0}\n"
		"5 => 7:1,5.{0}\n"
		"6 => 7:1,6.{0}"
		, "[^. ]+"
	);
	wait_for_match(60, expected, [&]()
	{
		return cluster[2].local_list();
	});

	expected = fmt::format(
		"3 => 7:1,3.{0}\n"
		"4 => 7:1,4.{0}\n"
		"5 => 7:1,5.{0}\n"
		"6 => 7:1,6.{0}"
		, "[^. ]+"
	);
	wait_for_match(40, expected, [&]()
	{
		return cluster[3].local_list();
	});

	expected = fmt::format(
		"4 => 7:1,4.{0}\n"
		"5 => 7:1,5.{0}\n"
		"6 => 7:1,6.{0}"
		, "[^. ]+"
	);
	wait_for_match(20, expected, [&]()
	{
		return cluster[4].local_list();
	});

	expected = fmt::format(
		"5 => 7:1,5.{0}\n"
		"6 => 7:1,6.{0}"
		, "[^. ]+"
	);
	wait_for_match(20, expected, [&]()
	{
		return cluster[5].local_list();
	});

	expected = fmt::format(
		"2 => 7:1,2.{0}\n"
		"3 => 7:1,3.{0}\n"
		"4 => 7:1,4.{0}\n"
		"5 => 7:1,5.{0}\n"
		"6 => 7:1,6.{0}"
		, "[^. ]+"
	);
	wait_for_match(20, expected, [&]()
	{
		return cluster[6].local_list();
	});
}
