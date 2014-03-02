/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "command_line/CommandLine.h"
#include "time/WaitFor.h"
using std::string;

TEST_CASE( "VersionSyncTest/testVersionsPropagate", "[integration]" )
{
	TurboCluster cluster(2, "--clone --no-write-chaining");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string filename = "conflict";
	// write different versions to multiple machines at once
	string response = CommandLine::run("echo 'write|name=" + filename + "|first' | nc -U " + cluster[1].dataChannel());
	response = CommandLine::run("echo 'write|name=" + filename + "|second' | nc -U " + cluster[2].dataChannel());

	// wait for propagation
	string expected = "(conflict)=>6|1,1:1 7|1,2:1";
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list();
		return expected == response;
	});

	// same keys stored in a different order
	expected = "(conflict)=>7|1,2:1 6|1,1:1";
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});
}

