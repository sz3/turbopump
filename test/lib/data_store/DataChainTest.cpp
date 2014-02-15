/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataChain.h"
#include "serialize/StringUtil.h"
#include <sstream>
#include <string>
using std::shared_ptr;
using std::string;

namespace {
	class TestableDataChain : public DataChain
	{
	public:
		using DataChain::bestVersion;
		using DataChain::_entries;
	};

	template <typename ... Args>
	shared_ptr<DataEntry> makeEntry(Args ... versions)
	{
		shared_ptr<DataEntry> entry(new DataEntry);

		std::vector<std::string> all{versions...};
		for (auto it = all.begin(); it != all.end(); ++it)
			entry->md.version.increment(*it);
		return entry;
	}

	std::ostream& operator<<(std::ostream& outstream, const bounded_version_vector<string>::clock& clock)
	{
		outstream << clock.key << ":" << clock.count;
		return outstream;
	}
}

TEST_CASE( "DataChainTest/testBestVersion", "[unit]" )
{
	TestableDataChain chain;

	chain._entries.push_back(makeEntry("foo"));
	chain._entries.push_back(makeEntry("bar"));
	chain._entries.push_back(makeEntry("bar", "foo", "foo"));
	chain._entries.push_back(makeEntry("rab"));

	assertEquals( "foo:2 rab:1 bar:1", StringUtil::join( chain.bestVersion().clocks() ) );
}
