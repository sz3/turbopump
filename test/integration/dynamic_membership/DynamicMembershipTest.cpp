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
using namespace turbo;

TEST_CASE( "DynamicMembershipTest/testGrow", "[integration]" )
{
	TurboRunner one(9001, "--udp --sync-interval 250");
	one.start();
	assertTrue( one.waitForRunning() );

	std::vector<string> fileList;
	fileList.push_back(".membership/9001 => 14:1,9001.[^. ]+");

	string expected = str::join(fileList, '\n');
	wait_for_match(5, expected, [&]()
	{
		return one.local_list("all=1");
	});

	TurboRunner two(9002, "--udp");
	two.start();
	assertTrue( two.waitForRunning() );

	string response = one.post("add-peer", "uid=9002&ip=127.0.0.1:9002");
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
	fileList.push_back(".membership/9002 => 14:1,9002.[^. ]+");
	expected = str::join(fileList, '\n');
	wait_for_match(20, expected, [&]()
	{
		return two.local_list("all=1");
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
	wait_for_equal(30, expectedMembers, [&]()
	{
		return three.query("membership");
	});
	wait_for_equal(30, expectedMembers, [&]()
	{
		return two.query("membership");
	});

	// test for member keys
	fileList.push_back(".membership/9003 => 14:1,9003.[^. ]+");
	expected = str::join(fileList, '\n');
	wait_for_match(100, expected, [&]()
	{
		return three.local_list("all=1");
	});
}

TEST_CASE( "DynamicMembershipTest/testGrow.FilesSpread", "[integration]" )
{
	TurboRunner one(9001, "--udp --sync-interval 250");
	one.start();
	assertTrue( one.waitForRunning() );

	std::vector<string> fileList;
	string response;
	for (unsigned i = 0; i < 50; ++i)
	{
		string name = str::str(i);
		string contents = "hello" + name;
		response = one.write(name, contents, "copies=3");
		assertEquals( "200", response );

		fileList.push_back(name + " => " + str::str(contents.size()) + ":1,9001.[^. ]+");
	}

	std::sort(fileList.begin(), fileList.end());
	string expected = str::join(fileList, '\n');
	wait_for_match(5, expected, [&]()
	{
		return one.local_list();
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
	expected = str::join(fileList, '\n');
	wait_for_match(60, expected, [&]()
	{
		return two.local_list();
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
	wait_for_equal(60, expectedMembers, [&]()
	{
		return three.query("membership");
	});
	wait_for_equal(60, expectedMembers, [&]()
	{
		return two.query("membership");
	});

	// and keys should propagate to three
	expected = str::join(fileList, '\n');
	wait_for_match(100, expected, [&]()
	{
		return three.local_list();
	});
}

