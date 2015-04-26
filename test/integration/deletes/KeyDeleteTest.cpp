/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "time/wait_for.h"
using std::string;
using turbo::wait_for_equal;
using turbo::wait_for_match;

TEST_CASE( "KeyDeleteTest/testDelete", "[integration-udp]" )
{
	TurboCluster cluster(2, "--udp");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string response = cluster[1].write("deleteMe", "hello");

	// wait for file
	string expected = "deleteMe => 5:1,1.([^. ]+)";
	string version = wait_for_match(10, expected, [&]()
	{
		return cluster[1].local_list();
	});
	wait_for_match(10, expected, [&]()
	{
		return cluster[2].local_list();
	});

	// delete file
	response = cluster[1].query("delete", "name=deleteMe&version=1,1." + version);
	expected = "";
	wait_for_equal(10, expected, [&]()
	{
		return cluster[1].local_list();
	});
	wait_for_equal(10, expected, [&]()
	{
		return cluster[2].local_list();
	});

	expected = "deleteMe => 9:2,delete." + version + ",1." + version;
	wait_for_equal(10, expected, [&]()
	{
		return cluster[1].local_list("deleted=1");
	});
	wait_for_equal(10, expected, [&]()
	{
		return cluster[2].local_list("deleted=1");
	});
}
