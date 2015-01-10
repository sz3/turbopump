/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "time/wait_for.h"
using std::string;

TEST_CASE( "RestartTest/testSync", "[integration-udp]" )
{
	TurboCluster cluster(3, "--udp --no-write-chaining");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	// shut down 2 workers
	assertTrue( cluster[2].stop(5) );
	assertTrue( cluster[3].stop(5) );

	// write files to running worker
	string response = cluster[1].write("foo", "hello!", "copies=2");
	assertEquals( "200", response );

	response = cluster[1].write("bar", "bar?", "copies=3");
	assertEquals( "200", response );

	// stop last worker.
	assertTrue( cluster[1].stop(5) );

	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	// wait for files to propagate
	wait_for(5, response, [&]()
	{
		response = cluster[1].local_list();
		return "bar => 4|1,1:1\n"
			   "foo => 6|1,1:1" == response;
	});

	wait_for(60, response, [&]()
	{
		response = cluster[2].local_list();
		return "bar => 4|1,1:1" == response;
	});

	wait_for(20, response, [&]()
	{
		response = cluster[3].local_list();
		return "bar => 4|1,1:1\n"
			   "foo => 6|1,1:1" == response;
	});
}
