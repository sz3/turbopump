/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DigestTree.h"

#include "KeyRange.h"
#include "hashing/Hash.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "DigestTreeTest/testBasics", "[unit]" )
{
	DigestTree tree;
	assertTrue( tree.empty() );

	tree.update("one", 0);
	tree.update("two", 0);
	tree.update("three", 0);
	assertFalse( tree.empty() );

	deque<string> files = tree.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", StringUtil::join(files) );

	tree.remove("two");
	tree.remove("three");

	files = tree.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", StringUtil::join(files) );

	tree.update("four", 0);
	files = tree.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", StringUtil::join(files) );

	tree.remove("one");
	tree.remove("four");
	files = tree.enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::join(files) );

	assertTrue( tree.empty() );
}

TEST_CASE( "DigestTreeTest/testTop", "[unit]" )
{
	DigestTree tree;

	tree.update("one", 0);
	unsigned long long hash1 = Hash("one").integer();
	MerklePoint top = tree.top();
	assertEquals( hash1, top.hash );

	tree.update("two", 0);
	unsigned long long hash2 = Hash("two").integer();
	top = tree.top();
	assertEquals( (hash1 xor hash2), top.hash );

	tree.update("three", 0);
	unsigned long long hash3 = Hash("three").integer();
	top = tree.top();
	assertEquals( (hash1 xor hash2 xor hash3), top.hash );
}

TEST_CASE( "DigestTreeTest/testUpdateExistingKey", "[unit]" )
{
	DigestTree tree;

	unsigned long long hash1 = Hash("one").integer();
	tree.update("one", 0x1234);
	MerklePoint top = tree.top();
	assertEquals( (hash1 xor 0x1234), top.hash );

	// update existing key
	hash1 = hash1 xor 0x5678;
	tree.update("one", 0x5678);
	top = tree.top();
	assertEquals( hash1, top.hash );

	unsigned long long hash2 = Hash("two").integer();
	tree.update("two", 0x4321);
	top = tree.top();
	assertEquals( (hash1 xor hash2 xor 0x4321), top.hash );

	hash2 = hash2 xor 0x8765;
	tree.update("two", 0x8765);
	top = tree.top();
	assertEquals( (hash1 xor hash2), top.hash );
}

TEST_CASE( "DigestTreeTest/testTraverse_Case1", "[unit]" )
{
	// make a generic test to iterate over two DigestTreees recursively,
	// and enforce that the appropriate missing key ranges are pushed to a list

	DigestTree treeOne;
	DigestTree treeTwo;

	treeOne.update("one0", 0);
	treeOne.update("one1", 0);
	treeOne.update("one2", 0);
	treeOne.update("one3", 0);
	treeOne.update("one4", 0);
	treeOne.update("two1", 0);
	treeOne.update("two2", 0);
	treeOne.update("two3", 0);
	treeOne.update("two4", 0);

	treeTwo.update("one0", 0);
	treeTwo.update("one1", 0);
	treeTwo.update("one2", 0);
	treeTwo.update("one3", 0);
	treeTwo.update("one4", 0);
	treeTwo.update("two0", 0);
	treeTwo.update("two1", 0);
	treeTwo.update("two2", 0);
	treeTwo.update("two3", 0);
	treeTwo.update("two4", 0);

	// start on two
	MerklePoint point = treeTwo.top();
	std::cout << " treeTwo.top = " << MerklePointSerializer::toString(point) << std::endl;

	MerklePoint p2 = treeOne.top();
	std::cout << " treeOne.top = " << MerklePointSerializer::toString(p2) << std::endl;

	// request diffs from two
	deque<MerklePoint> diffsTwo = treeTwo.diff(p2);
	for (deque<MerklePoint>::const_iterator it = diffsTwo.begin(); it != diffsTwo.end(); ++it)
		std::cout << " diffsTwo = " << MerklePointSerializer::toString(*it) << std::endl;

	// request diffs from one
	deque<MerklePoint> diffsOne = treeOne.diff(diffsTwo[0]);
	for (deque<MerklePoint>::const_iterator it = diffsOne.begin(); it != diffsOne.end(); ++it)
		std::cout << " diffsOne = " << MerklePointSerializer::toString(*it) << std::endl;

	/*diffsOne = treeOne.diff(diffsTwo[0]);
	for (deque<MerklePoint>::const_iterator it = diffsOne.begin(); it != diffsOne.end(); ++it)
		std::cout << " diffsOne = " << MerklePointSerializer::toString(*it) << std::endl;
	//*/

	KeyRange range(diffsOne[0].location);

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
	assertEquals( "two0", StringUtil::join(treeTwo.enumerate(range.first(), range.last())) );
}

