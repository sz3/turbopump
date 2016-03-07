/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "callbacks/Watches.h"

#include "serialize/format.h"
#include <functional>
#include <map>
#include <string>
using std::map;
using std::string;

namespace {
	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}
#include "serialize/str_join.h"

TEST_CASE( "WatchesTest/testNotify", "[unit]" )
{
	Watches watches;

	unsigned calls = 0;
	std::function<void()> callback = [&] ()
	{
		calls++;
	};
	std::string watchId = watches.watch("foo", callback);
	assertEquals(0, calls);

	assertFalse( watches.notify("bar") );
	assertEquals(0, calls);

	assertTrue( watches.notify("foo") );
	assertEquals(1, calls);

	assertFalse( watches.unwatch("foo", "wooops") );
	assertTrue( watches.notify("foo") );
	assertEquals(2, calls);

	assertTrue( watches.unwatch("foo", watchId) );
	assertFalse( watches.notify("foo") );
	assertEquals(2, calls);
}

TEST_CASE( "WatchesTest/testLoad", "[unit]" )
{
	Watches watches;

	map<string,unsigned> calls;
	for (unsigned i = 0; i < 1000; ++i)
	{
		string w = fmt::format("watch{}", i);
		auto fun = [w, &calls] ()
		{
			std::pair<map<string,unsigned>::iterator, bool> pear = calls.insert(std::pair<string,unsigned>(w, 0));
			pear.first->second++;
		};

		unsigned howmany = (i % 4) + 1;
		for (unsigned count = 0; count < howmany; ++count)
			watches.watch(w, fun);
	}

	watches.notify("bar");
	watches.notify("watch1");
	watches.notify("watch31");

	assertEquals("watch1=2 watch31=4", turbo::str::join(calls));
}

