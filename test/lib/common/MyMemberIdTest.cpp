/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MyMemberId.h"

TEST_CASE( "MyMemberIdTest/testDefault", "[unit]" )
{
	{
		MyMemberId id;
		assertEquals( "", id.str() );
	}

	{
		MyMemberId id("foo");
		assertEquals( "foo", id.str() );
		std::string idStr = id;
		assertEquals( "foo", idStr );
	}

	{
		MyMemberId id;
		assertEquals( "foo", id.str() );
	}

	{
		MyMemberId id("bar");
		assertEquals( "bar", id.str() );
	}

	MyMemberId id1;
	assertEquals( "bar", id1.str() );

	MyMemberId id2;
	assertEquals( "bar", id2.str() );

	assertEquals( &(id1.str()), &(id2.str()) );
}
