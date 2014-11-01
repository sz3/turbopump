/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "UniversalDigestIndexer.h"
#include "serialize/str_join.h"
#include <deque>
using std::deque;
using std::string;

TEST_CASE( "UniversalDigestIndexerTest/testBasics", "[unit]" )
{
	UniversalDigestIndexer index;

	index.update("one", 0);
	index.update("two", 0);
	index.update("three", 0);

	deque<string> files = index.find("whocares").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", turbo::str::join(files) );

	index.cannibalizeSection("ignore");
	index.splitSection("ignore");

	files = index.find("stilldoesn'tmatter").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", turbo::str::join(files) );

	files = index.randomTree().enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", turbo::str::join(files) );

	files = index.unwantedTree().enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "", turbo::str::join(files) );

	assertEquals( "", index.randomTree().id().id );
	assertEquals( 0, index.randomTree().id().mirrors );
}
