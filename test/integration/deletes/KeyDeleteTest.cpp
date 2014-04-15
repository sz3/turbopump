/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "integration/TurboCluster.h"
#include "integration/TurboRunner.h"
#include "serialize/StringUtil.h"
#include "time/WaitFor.h"
using std::string;

TEST_CASE( "KeyDeleteTest/testDelete", "[integration-udp]" )
{
	TurboCluster cluster(2, "--udp");
	cluster.start();
	assertMsg( cluster.waitForRunning(), cluster.lastError() );

	string response = CommandLine::run("echo 'write|name=deleteMe n=3|hello' | nc -U " + cluster[1].dataChannel());

	// wait for file
	string expected = "(deleteMe)=>6|1,1:1";
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list();
		return expected == response;
	});
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});

	// delete file
	response = CommandLine::run("echo 'delete|name=deleteMe v=1,1:1|' | nc -U " + cluster[1].dataChannel());
	expected = "";
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list();
		return expected == response;
	});
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list();
		return expected == response;
	});

	expected = "(deleteMe)=>9|2,delete:1,1:1";
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[1].local_list("deleted=true");
		return expected == response;
	});
	waitFor(10, expected + " != " + response, [&]()
	{
		response = cluster[2].local_list("deleted=true");
		return expected == response;
	});
}
