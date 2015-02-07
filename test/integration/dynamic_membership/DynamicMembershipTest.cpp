/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "integration/TurboRunner.h"
#include "membership/Membership.h"

#include "command_line/CommandLine.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include "time/wait_for.h"
#include <algorithm>
using std::string;

TEST_CASE( "DynamicMembershipTest/testGrow", "[integration]" )
{
	TurboRunner one(9001, "--udp");
	one.start();
	assertTrue( one.waitForRunning() );

	std::vector<string> fileList;
	fileList.push_back(".membership/9001 => 14|1,9001:1");

	string expected = turbo::str::join(fileList, '\n');
	string response;
	wait_for(5, expected + " != " + response, [&]()
	{
		response = one.local_list("all=1");
		return expected == response;
	});

	std::cerr << "starting worker two" << std::endl;

	TurboRunner two(9002, "--udp");
	two.start();
	assertTrue( two.waitForRunning() );

	response = one.post("add-peer", "uid=9002&ip=127.0.0.1:9002");
	assertEquals( "200", response );
	response = one.query("membership");
	assertEquals( "9001 127.0.0.1:9001\n"
				  "9002 127.0.0.1:9002", response );

	response = two.post("add-peer", "uid=9001&ip=127.0.0.1:9001");
	assertEquals( "200", response );
	response = two.query("membership");
	assertEquals( "9001 127.0.0.1:9001\n"
				  "9002 127.0.0.1:9002", response );

	// test for member keys
	fileList.push_back(".membership/9002 => 14|1,9002:1");
	expected = turbo::str::join(fileList, '\n');
	wait_for(20, expected + " != " + response, [&]()
	{
		response = two.local_list("all=1");
		return expected == response;
	});

	std::cerr << "starting worker three" << std::endl;

	TurboRunner three(9003, "--udp");
	three.start();
	assertTrue( three.waitForRunning() );

	response = one.post("add-peer", "uid=9003&ip=127.0.0.1:9003");
	string expectedMembers = "9001 127.0.0.1:9001\n"
							 "9002 127.0.0.1:9002\n"
							 "9003 127.0.0.1:9003";
	response = one.query("membership");
	assertEquals( expectedMembers, response );

	// tell 3 to join
	response = three.post("add-peer", "uid=9001&ip=127.0.0.1:9001");
	// membership changes should propagate to all members
	wait_for(30, expectedMembers + " != " + response, [&]()
	{
		response = three.query("membership");
		return expectedMembers == response;
	});
	wait_for(30, expectedMembers + " != " + response, [&]()
	{
		response = two.query("membership");
		return expectedMembers == response;
	});

	// test for member keys
	fileList.push_back(".membership/9003 => 14|1,9003:1");
	expected = turbo::str::join(fileList, '\n');
	wait_for(100, expected + " != " + response, [&]()
	{
		response = three.local_list("all=1");
		return expected == response;
	});
}

TEST_CASE( "DynamicMembershipTest/testGrow.FilesSpread", "[integration]" )
{
	TurboRunner one(9001, "--udp");
	one.start();
	assertTrue( one.waitForRunning() );

	std::vector<string> fileList;
	string response;
	for (unsigned i = 0; i < 50; ++i)
	{
		string name = turbo::str::str(i);
		string contents = "hello" + name;
		response = one.write(name, contents, "copies=3");
		assertEquals( "200", response );

		fileList.push_back(name + " => " + turbo::str::str(contents.size()) + "|1,9001:1");
	}

	std::sort(fileList.begin(), fileList.end());
	string expected = turbo::str::join(fileList, '\n');
	wait_for(5, expected + " != " + response, [&]()
	{
		response = one.local_list();
		return expected == response;
	});

	TurboRunner two(9002, "--udp");
	two.start();
	assertTrue( two.waitForRunning() );

	response = one.post("add-peer", "uid=9002&ip=127.0.0.1:9002");
	response = one.query("membership");
	assertEquals( "9001 127.0.0.1:9001\n"
				  "9002 127.0.0.1:9002", response );

	response = two.post("add-peer", "uid=9001&ip=127.0.0.1:9001");
	response = two.query("membership");
	assertEquals( "9001 127.0.0.1:9001\n"
				  "9002 127.0.0.1:9002", response );

	// keys should propagate to two
	expected = turbo::str::join(fileList, '\n');
	wait_for(60, expected + " != " + response, [&]()
	{
		response = two.local_list();
		return expected == response;
	});

	TurboRunner three(9003, "--udp");
	three.start();
	assertTrue( three.waitForRunning() );

	response = one.post("add-peer", "uid=9003&ip=127.0.0.1:9003");
	string expectedMembers = "9001 127.0.0.1:9001\n"
							 "9002 127.0.0.1:9002\n"
							 "9003 127.0.0.1:9003";
	response = one.query("membership");
	assertEquals( expectedMembers, response );

	// tell 3 to join
	response = three.post("add-peer", "uid=9001&ip=127.0.0.1:9001");
	// membership changes should propagate to all members
	wait_for(60, expectedMembers + " != " + response, [&]()
	{
		response = three.query("membership");
		return expectedMembers == response;
	});
	wait_for(60, expectedMembers + " != " + response, [&]()
	{
		response = two.query("membership");
		return expectedMembers == response;
	});

	// and keys should propagate to three
	expected = turbo::str::join(fileList, '\n');
	wait_for(100, expected + " != " + response, [&]()
	{
		response = three.local_list();
		return expected == response;
	});
}

