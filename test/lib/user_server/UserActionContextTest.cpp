/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "UserActionContext.h"
#include "mock/MockUserPacketHandler.h"
using std::string;

TEST_CASE( "UserActionContextTest/testDefault", "[unit]" )
{
	MockUserPacketHandler handler;
	UserActionContext context(handler);

	string buff = "GET /local_list?deleted=true&all=true HTTP/1.1\r\n\r\n";
	assertTrue( context.feed(buff.data(), buff.size()) );

	assertEquals( "newAction(local_list,all=true deleted=true)", handler._history.calls() );
}

