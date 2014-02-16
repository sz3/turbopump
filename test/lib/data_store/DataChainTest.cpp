/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataChain.h"
#include "common/MyMemberId.h"
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

	std::ostream& operator<<(std::ostream& outstream, const VectorClock::clock& clock)
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

TEST_CASE( "DataChainTest/testCreateVersion", "[unit]" )
{
	DataChain chain;

	{
		VectorClock clock;
		clock.increment("foo");
		shared_ptr<DataEntry> entry = chain.createVersion(clock);
		entry->data = "data!";
		entry->md.totalCopies = 2;
	}

	{
		VectorClock clock;
		clock.increment("bar");
		shared_ptr<DataEntry> entry = chain.createVersion(clock);
		entry->data = "other!";
		entry->md.totalCopies = 3;
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 2, entries.size() );

	assertEquals( "data!", entries[0]->data );
	assertEquals( 2, entries[0]->md.totalCopies );
	assertEquals( "foo:1", StringUtil::join(entries[0]->md.version.clocks()) );

	assertEquals( "other!", entries[1]->data );
	assertEquals( 3, entries[1]->md.totalCopies );
	assertEquals( "bar:1", StringUtil::join(entries[1]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testCreate", "[unit]" )
{
	MyMemberId("me");
	DataChain chain;

	{
		shared_ptr<DataEntry> entry = chain.create();
		entry->data = "data!";
		entry->md.totalCopies = 2;
	}

	{
		shared_ptr<DataEntry> entry = chain.create();
		entry->data = "other!";
		entry->md.totalCopies = 3;
	}

	{
		VectorClock clock;
		clock.increment("conflict");
		shared_ptr<DataEntry> entry = chain.createVersion(clock);
		entry->data = "conflict!";
		entry->md.totalCopies = 3;
	}


	{
		shared_ptr<DataEntry> entry = chain.create();
		entry->data = "four!";
		entry->md.totalCopies = 4;
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 4, entries.size() );

	assertEquals( "data!", entries[0]->data );
	assertEquals( 2, entries[0]->md.totalCopies );
	assertEquals( "me:1", StringUtil::join(entries[0]->md.version.clocks()) );

	assertEquals( "other!", entries[1]->data );
	assertEquals( 3, entries[1]->md.totalCopies );
	assertEquals( "me:2", StringUtil::join(entries[1]->md.version.clocks()) );

	assertEquals( "conflict!", entries[2]->data );
	assertEquals( 3, entries[2]->md.totalCopies );
	assertEquals( "conflict:1", StringUtil::join(entries[2]->md.version.clocks()) );

	assertEquals( "four!", entries[3]->data );
	assertEquals( 4, entries[3]->md.totalCopies );
	assertEquals( "me:3 conflict:1", StringUtil::join(entries[3]->md.version.clocks()) );
}
