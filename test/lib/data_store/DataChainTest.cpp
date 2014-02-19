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

TEST_CASE( "DataChainTest/testStore", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.version.increment("foo");
		chain.store(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("bar");
		chain.store(entry);
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

TEST_CASE( "DataChainTest/testStoreAsBestVersion", "[unit]" )
{
	MyMemberId("me");
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		chain.storeAsBestVersion(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		chain.storeAsBestVersion(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "conflict!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("conflict");
		chain.store(entry);
	}


	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "four!";
		entry->md.totalCopies = 4;
		chain.storeAsBestVersion(entry);
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

TEST_CASE( "DataChainTest/testFind", "[unit]" )
{
	DataChain chain;

	shared_ptr<DataEntry> found = chain.find(VectorClock());
	assertTrue( !found );

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.version.increment("foo");
		chain.store(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("bar");
		chain.store(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "third!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("foo");
		entry->md.version.increment("bar");
		chain.store(entry);
	}

	VectorClock version;
	found = chain.find(version);
	assertTrue( !found );

	version.increment("foo");
	found = chain.find(version);
	assertEquals( "data!", found->data );
	assertEquals( 2, found->md.totalCopies );
	assertEquals( "foo:1", StringUtil::join(found->md.version.clocks()) );

	version.clear();
	version.increment("bar");
	found = chain.find(version);
	assertEquals( "other!", found->data );
	assertEquals( 3, found->md.totalCopies );
	assertEquals( "bar:1", StringUtil::join(found->md.version.clocks()) );

	version.clear();
	version.increment("foo");
	version.increment("bar");
	found = chain.find(version);
	assertEquals( "third!", found->data );
	assertEquals( 3, found->md.totalCopies );
	assertEquals( "bar:1 foo:1", StringUtil::join(found->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testErase", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.version.increment("foo");
		chain.store(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("bar");
		chain.store(entry);
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "third!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("foo");
		entry->md.version.increment("bar");
		chain.store(entry);
	}

	assertEquals( 3, chain.entries().size() );

	// remove nothing
	VectorClock version;
	assertEquals( 3, chain.erase(version) );
	assertEquals( 3, chain.entries().size() );

	// remove "foo"
	version.increment("foo");
	assertEquals( 2, chain.erase(version) );

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 2, entries.size() );

	assertEquals( "other!", entries[0]->data );
	assertEquals( 3, entries[0]->md.totalCopies );
	assertEquals( "bar:1", StringUtil::join(entries[0]->md.version.clocks()) );

	assertEquals( "third!", entries[1]->data );
	assertEquals( 3, entries[1]->md.totalCopies );
	assertEquals( "bar:1 foo:1", StringUtil::join(entries[1]->md.version.clocks()) );

	// remove "foo bar"
	version.increment("bar");
	assertEquals( 1, chain.erase(version) );

	entries = chain.entries();
	assertEquals( 1, entries.size() );

	assertEquals( "other!", entries[0]->data );
	assertEquals( 3, entries[0]->md.totalCopies );
	assertEquals( "bar:1", StringUtil::join(entries[0]->md.version.clocks()) );

	version.clear();
	// remove "bar"
	version.increment("bar");
	assertEquals( 0, chain.erase(version) );
	assertEquals( 0, chain.entries().size() );

	// remove nothing
	assertEquals( 0, chain.erase(version) );
	assertEquals( 0, chain.entries().size() );
}
