/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AddPeerAction.h"

#include "common/DataBuffer.h"
#include "mock/MockAction.h"
#include <memory>
using std::map;
using std::string;

TEST_CASE( "AddPeerActionTest/testAdd", "[unit]" )
{
	std::unique_ptr<MockAction> food(new MockAction);
	MockAction& mockWriter = *food;
	AddPeerAction action(std::move(food));

	map<string,string> params;
	params["uid"] = "fooid";
	params["ip"] = "localhost:9001";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "setParams(n=0 name=.membership/fooid)|run(localhost:9001)", mockWriter._history.calls() );
}
