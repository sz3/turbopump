/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ActionParser.h"
#include "DataBuffer.h"
#include "serialize/StringUtil.h"
using std::string;

namespace
{
	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}

TEST_CASE( "ActionParserTest/testParseBuffer", "" )
{
	{
		ActionParser parser;
		DataBuffer buffer("I am an action!", 15);
		assertFalse( parser.parse(buffer) );
		assertEquals( "no action specified", parser.lastError() );
		assertEquals( 15, buffer.size() );
	}
	{
		ActionParser parser;
		DataBuffer buffer("I am an action!|I am params!", 28);
		assertFalse( parser.parse(buffer) );
		assertEquals( "no param end token specified for action I am an action!", parser.lastError() );
		assertEquals( 28, buffer.size() );
	}
	{
		ActionParser parser;
		DataBuffer buffer("I am an action!|I am params!|", 29);
		assertMsg( parser.parse(buffer), parser.lastError() );

		assertEquals( "I am an action!", parser.action() );
		assertEquals( 0, parser.params().size() );
		assertEquals( "", StringUtil::join(parser.params()) );
		assertEquals( 0, buffer.size() );
	}
	{
		ActionParser parser;
		DataBuffer buffer("I am an action!|foo=bar thing=what|data!", 40);
		assertMsg( parser.parse(buffer), parser.lastError() );

		assertEquals( "I am an action!", parser.action() );
		assertEquals( 2, parser.params().size() );
		assertEquals( "foo=bar thing=what", StringUtil::join(parser.params()) );

		assertEquals( 5, buffer.size() );
		assertEquals( "data!", buffer.str() );
	}
}


