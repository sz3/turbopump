/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DeleteAction.h"

#include "common/DataBuffer.h"
#include "common/VectorClock.h"
#include "mock/MockAction.h"
#include <memory>
using std::map;
using std::string;

TEST_CASE( "DeleteActionTest/testDelete", "[unit]" )
{
	std::unique_ptr<MockAction> food(new MockAction);
	MockAction& mockWriter = *food;
	DeleteAction action(std::move(food));

	VectorClock version;
	version.increment("foo");

	map<string,string> params;
	params["name"] = "baleeted";
	params["v"] = version.toString();
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "setParams(name=baleeted v=2,delete:1,foo:1)|run(timestamp)", mockWriter._history.calls() );
}
