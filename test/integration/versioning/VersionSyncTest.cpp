/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "common/VectorClock.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "cppformat/format.h"
#include "time/wait_for.h"
using std::string;
using namespace turbo;

TEST_CASE( "VersionSyncTest/testVersionsPropagate", "[integration]" )
{
	TurboCluster cluster(2, "--clone --no-write-chaining");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string filename = "conflict";
	// write different versions to multiple machines at once
	VectorClock v1;
	v1.increment("foo");
	string response = cluster[1].write(filename, "first", "version=" + v1.toString());

	VectorClock v2;
	v2.increment("bar");
	response = cluster[2].write(filename, "second", "version=" + v2.toString());

	// wait for propagation
	string expected = fmt::format("conflict => 5:{0} 6:{1}", v1.toString(), v2.toString());
	wait_for_equal(10, expected, [&]()
	{
		return cluster[1].local_list();
	});

	wait_for_equal(10, expected, [&]()
	{
		return cluster[2].local_list();
	});
}

