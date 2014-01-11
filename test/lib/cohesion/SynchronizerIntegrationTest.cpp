/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ICorrectSkew.h"
#include "MerkleIndex.h"
#include "Synchronizer.h"
#include "actions_req/IMessageSender.h"
#include "membership/Peer.h"
#include "mock/MockMembership.h"
#include "serialize/StringUtil.h"
using std::deque;
using std::string;

namespace
{
	class TestMessageSender : public IMessageSender
	{
	public:
		void merklePing(const Peer& peer, const MerklePoint& point)
		{
			_other->compare(peer, point);
		}

		void merklePing(const Peer& peer, const deque<MerklePoint>& points)
		{
			for (deque<MerklePoint>::const_iterator it = points.begin(); it != points.end(); ++it)
				_other->compare(peer, *it);
		}

		void requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
		{
			_other->pushKeyRange(peer, first, last);
		}

	public:
		Synchronizer* _other;
	};

	class TestSkewCorrector : public ICorrectSkew
	{
	public:
		TestSkewCorrector(const MerkleIndex& index)
			: _index(index)
		{}

		void healKey(const Peer& peer, unsigned long long key)
		{
			_history.call("healKey", key);
		}

		void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
		{
			_history.call("pushKeyRange", first, last);
			deque<string> toPush = _index.enumerate(first, last);
			_corrected.insert(_corrected.end(), toPush.begin(), toPush.end());
		}

	public:
		const MerkleIndex& _index;
		deque<string> _corrected;
		CallHistory _history;
	};
}

TEST_CASE( "SynchronizerIntegrationTest/testCompareExchange", "[integration]" )
{
	MerkleIndex indexOne;
	MerkleIndex indexTwo;

	indexOne.add("one0");
	indexOne.add("one1");
	indexOne.add("one2");
	indexOne.add("one3");
	indexOne.add("one4");

	indexTwo.add("two0");
	indexTwo.add("two1");
	indexTwo.add("two2");
	indexTwo.add("two3");
	indexTwo.add("two4");

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

	//indexOne.print();
	//indexTwo.print();

	TestMessageSender senderOne;
	TestSkewCorrector correctorOne(indexOne);
	TestMessageSender senderTwo;
	TestSkewCorrector correctorTwo(indexTwo);

	MockMembership membership;
	Synchronizer one(membership, indexOne, senderOne, correctorOne);
	Synchronizer two(membership, indexTwo, senderTwo, correctorTwo);

	senderOne._other = &two;
	senderTwo._other = &one;

	one.compare(Peer("dummy"), indexTwo.top());

	//std::cout << "correctorOne says : " << correctorOne._history.calls() << std::endl;
	//std::cout << "correctorTwo says : " << correctorTwo._history.calls() << std::endl;
	assertEquals( "one0 one1 one2", StringUtil::stlJoin(correctorOne._corrected) );
	assertEquals( "two3 two2 two4 two1", StringUtil::stlJoin(correctorTwo._corrected) );
}

TEST_CASE( "SynchronizerIntegrationTest/testCompareExchange.Case2", "[integration]" )
{
	MerkleIndex indexOne;
	MerkleIndex indexTwo;

	for (int i = 0; i < 100; ++i)
	{
		string id = StringUtil::str(i);
		indexOne.add(id);
	}

	//indexOne.print(2);

	TestMessageSender senderOne;
	TestSkewCorrector correctorOne(indexOne);
	TestMessageSender senderTwo;
	TestSkewCorrector correctorTwo(indexTwo);

	MockMembership membership;
	Synchronizer one(membership, indexOne, senderOne, correctorOne);
	Synchronizer two(membership, indexTwo, senderTwo, correctorTwo);

	senderOne._other = &two;
	senderTwo._other = &one;

	one.compare(Peer("dummy"), indexTwo.top());

	//std::cout << "correctorOne says : " << correctorOne._history.calls() << std::endl;
	//std::cout << "correctorTwo says : " << correctorTwo._history.calls() << std::endl;
	assertEquals( "2 26 46 6 40 9 55 21 67 41 90 32 62 69 54 10 29 8 1 94 56 28 33 49 99 7 65"
				  " 13 88 71 25 58 0 74 64 61 53 66 57 47 27 60 5 97 4 17 14 20 48 63", StringUtil::stlJoin(correctorOne._corrected) );
	assertEquals( "", StringUtil::stlJoin(correctorTwo._corrected) );


	// set up trial two
	for (deque<string>::const_iterator it = correctorOne._corrected.begin(); it != correctorOne._corrected.end(); ++it)
		indexTwo.add(*it);

	correctorOne._corrected.clear();
	correctorOne._history.clear();
	correctorTwo._history.clear();

	two.compare(Peer("dummy"), indexOne.top());

	//std::cout << "correctorOne says : " << correctorOne._history.calls() << std::endl;
	//std::cout << "correctorTwo says : " << correctorTwo._history.calls() << std::endl;
	assertEquals( "36 79 68 75 44 92 11 77 51 24 16 52 91 23 87 38 30 39 93 45 35 3 70 15 37 18"
				  " 19 73 42 72 80 12 78 84 96 76 89 31 43 50 82 81 85 98 22", StringUtil::stlJoin(correctorOne._corrected) );
	assertEquals( "", StringUtil::stlJoin(correctorTwo._corrected) );

	for (deque<string>::const_iterator it = correctorOne._corrected.begin(); it != correctorOne._corrected.end(); ++it)
		indexTwo.add(*it);

	deque<string> filesOne = indexOne.enumerate(0,~0ULL,100);
	deque<string> filesTwo = indexTwo.enumerate(0,~0ULL,100);
	//assertEquals( StringUtil::stlJoin(filesOne), StringUtil::stlJoin(filesTwo) );
	assertEquals( 95, filesTwo.size() );
}
