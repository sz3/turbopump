/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "command_line/CommandLine.h"
#include "time/wait_for.h"
using std::string;

TEST_CASE( "VersionSyncTest/testVersionsPropagate", "[integration]" )
{
	TurboCluster cluster(2, "--clone --no-write-chaining");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string filename = "conflict";
	// write different versions to multiple machines at once
	string response = cluster[1].write(filename, "first", "version=1,foo:1");
	response = cluster[2].write(filename, "second", "version=1,bar:1");

	// wait for propagation
	string expected = "conflict => 5|1,foo:1 6|1,bar:1";
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list();
		return expected == response;
	});

	// same keys stored in a different order
	expected = "conflict => 6|1,bar:1 5|1,foo:1";
	wait_for(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});
}

