/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataChain.h"
#include "common/MyMemberId.h"
#include "hashing/Hash.h"
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
		using DataChain::clearLesser_unlocked;
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
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.supercede = false;
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
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

TEST_CASE( "DataChainTest/testStoreSameVersion", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertFalse( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, entries.size() );

	assertEquals( "data!", entries[0]->data );
	assertEquals( 2, entries[0]->md.totalCopies );
	assertEquals( "foo:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testStoreSameVersion.Append", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "hereismore!";
		entry->md.totalCopies = 3;
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 5) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "whoops, too far!";
		entry->md.totalCopies = 3;
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertFalse( chain.store(entry, 50) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, entries.size() );

	assertEquals( "data!hereismore!", entries[0]->data );
	assertEquals( 2, entries[0]->md.totalCopies );
	assertEquals( "foo:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testStore.Supercede", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
	}

	assertEquals( 2, chain.entries().size() );

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "superceder!";
		entry->md.version.increment("bar");
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, chain.entries().size() );

	assertEquals( "superceder!", entries[0]->data );
	assertEquals( "foo:1 bar:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testStore.AlreadyExists", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "foo!";
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "bar?";
		entry->md.version.increment("foo");
		assertFalse( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, chain.entries().size() );

	assertEquals( "foo!", entries[0]->data );
	assertEquals( "foo:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testStore.IsSuperceded", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "superceder!";
		entry->md.version.increment("bar");
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.version.increment("foo");
		assertFalse( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.version.increment("bar");
		assertFalse( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, chain.entries().size() );

	assertEquals( "superceder!", entries[0]->data );
	assertEquals( "foo:1 bar:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testStoreAsBestVersion", "[unit]" )
{
	MyMemberId("me");
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.totalCopies = 2;
		entry->md.supercede = false;
		assertTrue( chain.storeAsBestVersion(entry) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.totalCopies = 3;
		entry->md.supercede = false;
		assertTrue( chain.storeAsBestVersion(entry) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "conflict!";
		entry->md.totalCopies = 3;
		entry->md.supercede = false;
		entry->md.version.increment("conflict");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "four!";
		entry->md.totalCopies = 4;
		entry->md.supercede = false;
		assertTrue( chain.storeAsBestVersion(entry) );
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

TEST_CASE( "DataChainTest/testStoreAsBestVersion.Supercede", "[unit]" )
{
	MyMemberId("me");
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		assertTrue( chain.storeAsBestVersion(entry) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		assertTrue( chain.storeAsBestVersion(entry) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "conflict!";
		entry->md.version.increment("conflict");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "four!";
		assertTrue( chain.storeAsBestVersion(entry) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, entries.size() );
	assertEquals( "four!", entries[0]->data );
	assertEquals( "me:3 conflict:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testMarkDeleted", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "foo!";
		entry->md.version.increment("valid");
		assertTrue( chain.store(entry, 0) );

		entry.reset(new DataEntry);
		entry->data = "also deleted!";
		entry->md.totalCopies = 10; // ignored
		entry->md.version.increment("valid");
		entry->md.version.markDeleted();
		assertTrue( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, entries.size() );
	assertEquals( "also deleted!", entries[0]->data );
	assertEquals( 3, entries[0]->md.totalCopies );
	assertEquals( "delete:1 valid:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testMarkDeleted.NeverGotOriginal", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "deleted!";
		entry->md.totalCopies = 5;
		entry->md.version.markDeleted();
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->md.version.markDeleted();
		assertFalse( chain.store(entry, 0) ); // already exists.
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, entries.size() );
	assertEquals( "deleted!", entries[0]->data );
	assertEquals( 5, entries[0]->md.totalCopies );
	assertEquals( "delete:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testMarkDeleted.StoreFails", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "foo!";
		entry->md.version.increment("foo");
		entry->md.version.markDeleted();
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "bar?";
		entry->md.version.increment("foo");
		assertFalse( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, chain.entries().size() );

	assertEquals( "foo!", entries[0]->data );
	assertEquals( "delete:1 foo:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testMarkDeleted.OverwriteAndSupercede", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "foo!";
		entry->md.version.increment("foo");
		entry->md.version.markDeleted();
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "bar";
		entry->md.version.increment("foo");
		entry->md.version.increment("delete");
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
	}

	std::vector< shared_ptr<DataEntry> > entries = chain.entries();
	assertEquals( 1, chain.entries().size() );

	assertEquals( "bar", entries[0]->data );
	assertEquals( "bar:1 delete:1 foo:1", StringUtil::join(entries[0]->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testFind", "[unit]" )
{
	DataChain chain;

	shared_ptr<DataEntry> found = chain.find(VectorClock());
	assertTrue( !found );

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.supercede = false;
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "third!";
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
	}

	VectorClock version;
	found = chain.find(version);
	assertTrue( !found );

	version.increment("foo");
	found = chain.find(version);
	assertEquals( "data!", found->data );
	assertEquals( "foo:1", StringUtil::join(found->md.version.clocks()) );

	version.clear();
	version.increment("bar");
	found = chain.find(version);
	assertEquals( "other!", found->data );
	assertEquals( "bar:1", StringUtil::join(found->md.version.clocks()) );

	version.clear();
	version.increment("foo");
	version.increment("bar");
	found = chain.find(version);
	assertEquals( "third!", found->data );
	assertEquals( "bar:1 foo:1", StringUtil::join(found->md.version.clocks()) );
}

TEST_CASE( "DataChainTest/testErase", "[unit]" )
{
	DataChain chain;

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "data!";
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "other!";
		entry->md.supercede = false;
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
	}

	{
		shared_ptr<DataEntry> entry(new DataEntry);
		entry->data = "third!";
		entry->md.supercede = false;
		entry->md.version.increment("foo");
		entry->md.version.increment("bar");
		assertTrue( chain.store(entry, 0) );
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

TEST_CASE( "DataChainTest/testClearLesser.Some", "[unit]" )
{
	TestableDataChain chain;
	chain._entries.push_back(makeEntry("foo"));
	chain._entries.push_back(makeEntry("bar"));
	chain._entries.push_back(makeEntry("bar", "foo"));
	chain._entries.push_back(makeEntry("bar", "foo", "foo"));
	chain._entries.push_back(makeEntry("rab"));
	assertEquals( 5, chain.entries().size() );
	assertEquals( 5, chain.entries().size() );

	VectorClock version;
	version.increment("bar");
	version.increment("foo");
	assertEquals( 3, chain.clearLesser_unlocked(version) );
	assertEquals( 3, chain.entries().size() );
}

TEST_CASE( "DataChainTest/testClearLesser.Most", "[unit]" )
{
	TestableDataChain chain;
	chain._entries.push_back(makeEntry("foo"));
	chain._entries.push_back(makeEntry("bar"));
	chain._entries.push_back(makeEntry("bar", "foo"));
	chain._entries.push_back(makeEntry("bar", "foo", "foo"));
	chain._entries.push_back(makeEntry("rab"));
	assertEquals( 5, chain.entries().size() );
	assertEquals( 5, chain.entries().size() );

	VectorClock version;
	version.increment("bar");
	version.increment("bar");
	version.increment("foo");
	version.increment("foo");
	assertEquals( 1, chain.clearLesser_unlocked(version) );
	assertEquals( 1, chain.entries().size() );
}

TEST_CASE( "DataChainTest/testSummary", "[unit]" )
{
	TestableDataChain chain;
	chain._entries.push_back(makeEntry("foo"));
	chain._entries.back()->data = "four";

	{
		VectorClock version;
		version.increment("foo");
		assertEquals( Hash("4" + version.toString()).integer(), chain.summary() );
	}

	unsigned long long first = chain.summary();
	chain._entries.push_back(makeEntry("bar"));

	{
		VectorClock version;
		version.increment("bar");
		assertEquals( (Hash("0" + version.toString()).integer() xor first), chain.summary() );
	}
}

TEST_CASE( "DataChainTest/testSummary.Transitive", "[unit]" )
{
	TestableDataChain one;
	one._entries.push_back(makeEntry("foo"));
	one._entries.push_back(makeEntry("bar"));
	one._entries.push_back(makeEntry("foo", "bar", "bar"));

	TestableDataChain two;
	two._entries.push_back(makeEntry("foo", "bar", "bar"));
	two._entries.push_back(makeEntry("bar"));
	two._entries.push_back(makeEntry("foo"));

	assertEquals( one.summary(), two.summary() );
}
