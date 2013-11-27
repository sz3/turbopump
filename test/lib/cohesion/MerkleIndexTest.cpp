#include "unittest.h"

#include "MerkleIndex.h"
#include "consistent_hashing/Hash.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "MerkleIndexTest/testBasics", "[unit]" )
{
	MerkleIndex index;

	index.add("one");
	index.add("two");
	index.add("three");

	deque<string> files = index.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", StringUtil::stlJoin(files) );

	index.remove("two");
	index.remove("three");

	files = index.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", StringUtil::stlJoin(files) );

	index.add("four");
	files = index.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", StringUtil::stlJoin(files) );

	index.remove("one");
	index.remove("four");
	files = index.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::stlJoin(files) );
}

TEST_CASE( "MerkleIndexTest/testTop", "[unit]" )
{
	MerkleIndex index;

	index.add("one");
	unsigned long long hash1 = Hash::compute("one").integer();
	MerklePoint top = index.top();
	assertEquals( hash1, top.hash );

	index.add("two");
	unsigned long long hash2 = Hash::compute("two").integer();
	top = index.top();
	assertEquals( (hash1 xor hash2), top.hash );

	index.add("three");
	unsigned long long hash3 = Hash::compute("three").integer();
	top = index.top();
	assertEquals( (hash1 xor hash2 xor hash3), top.hash );
}

TEST_CASE( "MerkleIndexTest/testBadness", "[unit]" )
{
	MerkleIndex indexOne;
	MerkleIndex indexTwo;

	indexOne.add("one0");
	indexOne.add("one1");
	indexOne.add("one2");
	indexOne.add("one3");
	indexOne.add("one4");
	indexOne.add("two1");

	indexTwo.add("one0");
	indexTwo.add("one1");
	indexTwo.add("one2");
	indexTwo.add("one3");
	indexTwo.add("one4");
	indexTwo.add("two0");
	indexTwo.add("two1");
	indexTwo.add("two2");
	indexTwo.add("two3");
	indexTwo.add("two4");

	// start on two
	MerklePoint point = indexTwo.top();
	std::cout << " indexTwo.top = " << MerklePointSerializer::toString(point) << std::endl;

	// request diffs from one
	deque<MerklePoint> diffsOne = indexOne.diff(point);
	for (deque<MerklePoint>::const_iterator it = diffsOne.begin(); it != diffsOne.end(); ++it)
		std::cout << " diffsOne = " << MerklePointSerializer::toString(*it) << std::endl;

	// compare diffs back to two
	deque<MerklePoint> diffsTwo = indexTwo.diff(diffsOne[1]);
	for (deque<MerklePoint>::const_iterator it = diffsTwo.begin(); it != diffsTwo.end(); ++it)
		std::cout << " diffsTwo = " << MerklePointSerializer::toString(*it) << std::endl;

	// this diff == the same as the previous one. Uh oh!
	// we need a way to sanity check the returned diff -- perhaps a "parent keybits"? Alternatively, a persisted state...
	diffsOne = indexOne.diff(diffsTwo[0]);
	for (deque<MerklePoint>::const_iterator it = diffsOne.begin(); it != diffsOne.end(); ++it)
		std::cout << " diffsOne = " << MerklePointSerializer::toString(*it) << std::endl;
}
