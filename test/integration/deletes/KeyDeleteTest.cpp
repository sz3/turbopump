/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "time/wait_for.h"
using std::string;

TEST_CASE( "KeyDeleteTest/testDelete", "[integration-udp]" )
{
	TurboCluster cluster(2, "--udp");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string response = cluster[1].write("deleteMe", "hello");

	// wait for file
	string expected = "deleteMe => 5|1,1:1";
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list();
		return expected == response;
	});
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});

	// delete file
	response = cluster[1].query("delete", "name=deleteMe&version=1,1:1");
	expected = "";
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list();
		return expected == response;
	});
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});

	expected = "deleteMe => 9|2,delete:1,1:1";
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list("deleted=1");
		return expected == response;
	});
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list("deleted=1");
		return expected == response;
	});
}
