/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"

#include "command_line/CommandLine.h"
#include "time/WaitFor.h"
using std::string;

TEST_CASE( "VersionChainingTest/testCreateAndFixConflict", "[integration]" )
{
	TurboCluster cluster(2, "--clone --no-active-sync");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string filename = "conflict";
	// write different versions to multiple machines at once
	string response = CommandLine::run("echo 'write|name=" + filename + " v=1,foo:1|first' | nc -U " + cluster[1].dataChannel());
	response = CommandLine::run("echo 'write|name=" + filename + " v=1,bar:1|second' | nc -U " + cluster[2].dataChannel());

	// check for both versions
	string expected = "(conflict)=>6|1,foo:1 7|1,bar:1";
	assertEquals( expected, cluster[1].local_list() );
	assertEquals( expected, cluster[2].local_list() );

	// fix it
	response = CommandLine::run("echo 'write|name=" + filename + "|thereIfixedit' | nc -U " + cluster[1].dataChannel());

	expected = "(conflict)=>14|3,1:1,foo:1,bar:1";
	assertEquals( expected, cluster[1].local_list() );
	assertEquals( expected, cluster[2].local_list() );
}

