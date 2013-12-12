#include "unittest.h"

#include "MerkleIndex.h"
#include "MerkleRange.h"
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

TEST_CASE( "MerkleIndexTest/testTraverse_Case1", "[unit]" )
{
	// make a generic test to iterate over two MerkleIndexes recursively,
	// and enforce that the appropriate missing key ranges are pushed to a list

	MerkleIndex indexOne;
	MerkleIndex indexTwo;

	indexOne.add("one0");
	indexOne.add("one1");
	indexOne.add("one2");
	indexOne.add("one3");
	indexOne.add("one4");
	indexOne.add("two1");
	indexOne.add("two2");
	indexOne.add("two3");
	indexOne.add("two4");

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

	MerklePoint p2 = indexOne.top();
	std::cout << " indexOne.top = " << MerklePointSerializer::toString(p2) << std::endl;

	// request diffs from two
	deque<MerklePoint> diffsTwo = indexTwo.diff(p2);
	for (deque<MerklePoint>::const_iterator it = diffsTwo.begin(); it != diffsTwo.end(); ++it)
		std::cout << " diffsTwo = " << MerklePointSerializer::toString(*it) << std::endl;

	// request diffs from one
	deque<MerklePoint> diffsOne = indexOne.diff(diffsTwo[0]);
	for (deque<MerklePoint>::const_iterator it = diffsOne.begin(); it != diffsOne.end(); ++it)
		std::cout << " diffsOne = " << MerklePointSerializer::toString(*it) << std::endl;

	/*diffsOne = indexOne.diff(diffsTwo[0]);
	for (deque<MerklePoint>::const_iterator it = diffsOne.begin(); it != diffsOne.end(); ++it)
		std::cout << " diffsOne = " << MerklePointSerializer::toString(*it) << std::endl;
	//*/

	MerkleRange range(diffsOne[0].location);

	// one0: 0001 0011 | 0110 1101 | 0100 1011 | 1101 0000 | 1011 0111 | 1101 0101 | 1110 1111 | 1101 0100
	// one1: 0011 0001 | 0111 1011 | 1100 0001 | 0001 1101 | 1010 1110 | 0001 1111 | 0000 0111 | 1110 1010
	// one2: 0011 1100 | 0110 1110 | 1010 1010 | 1001 1101 | 1111 1000 | 0110 0110 | 0010 1011 | 0010 1110
	// two0: 0100 1110 | 1100 0101 | 0000 0110 | 1101 1001 | 0110 0111 | 1010 1100 | 1100 0101 | 0101 1001
	// one3: 1000 0010 | 0100 0010 | 0101 0010 | 1110 1111 | 1001 1000 | 0110 0010 | 1010 1000 | 1101 1000
	// two3: 1000 0010 | 1000 0100 | 1100 1100 | 1110 1001 | 0010 1101 | 0011 1010 | 0000 0001 | 1011 0111
	// two2: 1100 0100 | 0100 1111 | 1101 1010 | 1101 1010 | 1010 1110 | 0110 0111 | 0001 0100 | 1010 1111
	// two4: 1110 0111 | 0010 0110 | 0010 0110 | 0101 0011 | 1101 1001 | 0101 0101 | 1101 1100 | 0101 1010
	// one4: 1110 1000 | 1011 1011 | 0110 1001 | 1011 1001 | 0001 1010 | 0100 0001 | 1111 1000 | 0000 0011
	// two1: 1111 0101 | 1100 0011 | 0011 0110 | 0011 0001 | 0100 1111 | 0101 0001 | 0100 1011 | 1010 0010

	assertEquals(64, range.first());
	assertEquals(0xFFFFFFFFFFFFFF7FULL, range.last());
	assertEquals( "two0", StringUtil::stlJoin(indexTwo.enumerate(range.first(), range.last())) );
}

