/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboRunner.h"
#include "membership/Membership.h"

#include "command_line/CommandLine.h"
#include "serialize/StringUtil.h"
#include "time/WaitFor.h"
#include <algorithm>
using std::string;

TEST_CASE( "DynamicMembershipTest/testGrow", "[integration]" )
{
	TurboRunner one(9001, "--udp");
	one.start();
	assertTrue( one.waitForRunning() );

	std::vector<string> fileList;
	string response;
	for (unsigned i = 0; i < 50; ++i)
	{
		string name = StringUtil::str(i);
		string contents = "hello" + name;
		response = CommandLine::run("echo 'write|name=" + name + " n=3|" + contents + "' | nc -U " + one.dataChannel());
		assertEquals( "", response );
		fileList.push_back("(" + name + ")=>" + StringUtil::str(contents.size()+1));
	}

	std::sort(fileList.begin(), fileList.end());
	string expected = StringUtil::join(fileList, '\n');
	waitFor(5, expected + " != " + response, [&]()
	{
		response = one.query("local_list");
		return expected == response;
	});

	TurboRunner two(9002, "--udp");
	two.start();
	assertTrue( two.waitForRunning() );

	response = CommandLine::run("echo 'add_peer|uid=9002 ip=127.0.0.1:9002|' | nc -U " + one.dataChannel());
	response = one.query("membership");
	assertEquals( "9001 127.0.0.1:9001\n"
				  "9002 127.0.0.1:9002", response );

	response = CommandLine::run("echo 'add_peer|uid=9001 ip=127.0.0.1:9001|' | nc -U " + two.dataChannel());
	response = two.query("membership");
	assertEquals( "9001 127.0.0.1:9001\n"
				  "9002 127.0.0.1:9002", response );

	waitFor(20, expected + " != " + response, [&]()
	{
		response = two.query("local_list");
		return expected == response;
	});
}
