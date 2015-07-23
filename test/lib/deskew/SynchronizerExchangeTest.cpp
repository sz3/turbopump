/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ICorrectSkew.h"
#include "IDigestKeys.h"
#include "KeyTabulator.h"
#include "Synchronizer.h"
#include "TreeId.h"

#include "membership/Peer.h"
#include "mock/MockConsistentHashRing.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockLogger.h"
#include "mock/MockKnownPeers.h"
#include "peer_client/IMessageSender.h"

#include "serialize/str.h"
#include "serialize/str_join.h"
using std::deque;
using std::string;

namespace
{
	class TestSkewCorrector : public ICorrectSkew
	{
	public:
		TestSkewCorrector(const KeyTabulator& index)
			: _index(index)
		{}

		bool dropKey(const std::string& name)
		{
			_history.call("pushKey", name);
			return true;
		}

		void pushKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
		{
			_history.call("pushKey", treeid.id, key);
			deque<string> toHeal = _index.find(treeid.id, treeid.mirrors).enumerate(key, key);
			_healed.insert(_healed.end(), toHeal.begin(), toHeal.end());
		}

		void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const string& offloadFrom="")
		{
			_history.call("pushKeyRange", treeid.id, first, last, offloadFrom);
			deque<string> toPush = _index.find(treeid.id, treeid.mirrors).enumerate(first, last);
			_corrected.insert(_corrected.end(), toPush.begin(), toPush.end());
		}

		bool sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source)
		{
			_history.call("sendKey", name, version, source);
			return true;
		}

	public:
		const KeyTabulator& _index;
		deque<string> _corrected;
		deque<string> _healed;
		CallHistory _history;
	};

	class TestMessageSender : public IMessageSender
	{
	public:
		void digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point)
		{
			_other->compare(peer, treeid, point);
		}

		void digestPing(const Peer& peer, const TreeId& treeid, const deque<MerklePoint>& points)
		{
			for (unsigned i = 0; i < points.size(); ++i)
				_other->compare(peer, treeid, points[i], i >= 2);
		}

		void requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last)
		{
			_corrector->pushKeyRange(peer, treeid, first, last);
		}

		void requestKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
		{
			_corrector->pushKey(peer, treeid, key);
		}

		void offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
		{
		}

		void demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
		{
		}

		void acknowledgeWrite(const Peer& peer, const std::string& filename, const std::string& version, unsigned long long size)
		{
		}

	public:
		Synchronizer* _other;
		TestSkewCorrector* _corrector;
	};
}

TEST_CASE( "SynchronizerExchangeTest/testCompareExchange", "[integration]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;

	MockLocateKeys locator;
	locator._locations.push_back("fooid");
	KeyTabulator indexOne(locator);
	KeyTabulator indexTwo(locator);

	indexOne.update("one0", 0);
	indexOne.update("one1", 0);
	indexOne.update("one2", 0);
	indexOne.update("one3", 0);
	indexOne.update("one4", 0);

	indexTwo.update("two0", 0);
	indexTwo.update("two1", 0);
	indexTwo.update("two2", 0);
	indexTwo.update("two3", 0);
	indexTwo.update("two4", 0);

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

	MockLogger logger;
	Synchronizer one(ring, membership, indexOne, senderOne, correctorOne, logger);
	Synchronizer two(ring, membership, indexTwo, senderTwo, correctorTwo, logger);

	senderOne._other = &two;
	senderOne._corrector = &correctorTwo;
	senderTwo._other = &one;
	senderTwo._corrector = &correctorOne;

	one.compare(Peer("dummy"), TreeId("fooid"), indexTwo.find("fooid").top());

	//std::cout << "correctorOne says : " << correctorOne._history.calls() << std::endl;
	//std::cout << "correctorTwo says : " << correctorTwo._history.calls() << std::endl;

	// since we're not fixing anything, some keys are duplicated...
	assertEquals( "one0 one1 one2 one3 one4 one3 one4", turbo::str::join(correctorOne._corrected) );
	assertEquals( "two3 two2 two4 two1 two0 two3 two2 two4 two1", turbo::str::join(correctorTwo._corrected) );
}

TEST_CASE( "SynchronizerExchangeTest/testCompareExchange.Case2", "[integration]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;

	MockLocateKeys locator;
	locator._locations.push_back("fooid");
	KeyTabulator indexOne(locator);
	KeyTabulator indexTwo(locator);

	for (int i = 0; i < 100; ++i)
	{
		string id = turbo::str::str(i);
		indexOne.update(id, 0);
	}

	//indexOne.print(2);

	TestMessageSender senderOne;
	TestSkewCorrector correctorOne(indexOne);
	TestMessageSender senderTwo;
	TestSkewCorrector correctorTwo(indexTwo);

	MockLogger logger;
	Synchronizer one(ring, membership, indexOne, senderOne, correctorOne, logger);
	Synchronizer two(ring, membership, indexTwo, senderTwo, correctorTwo, logger);

	senderOne._other = &two;
	senderOne._corrector = &correctorTwo;
	senderTwo._other = &one;
	senderTwo._corrector = &correctorOne;

	one.compare(Peer("dummy"), TreeId("fooid"), indexTwo.find("fooid").top());

	//std::cout << "correctorOne says : " << correctorOne._history.calls() << std::endl;
	//std::cout << "correctorTwo says : " << correctorTwo._history.calls() << std::endl;
	assertEquals( "2 26 46 6 40 9 55 21 67 41 90 32 62 69 54 10 29 8 1 94 56 28 33 49 99 7 65"
				  " 13 88 71 25 58 0 74 64 61 53 66 57 47 27 60 5 97 4 17 14 20 48 63", turbo::str::join(correctorOne._corrected) );
	assertEquals( "", turbo::str::join(correctorTwo._corrected) );


	// set up trial two
	for (deque<string>::const_iterator it = correctorOne._corrected.begin(); it != correctorOne._corrected.end(); ++it)
		indexTwo.update(*it, 0);

	correctorOne._corrected.clear();
	correctorOne._history.clear();
	correctorTwo._history.clear();

	two.compare(Peer("dummy"), TreeId("fooid"), indexOne.find("fooid").top());

	//std::cout << "correctorOne says : " << correctorOne._history.calls() << std::endl;
	//std::cout << "correctorTwo says : " << correctorTwo._history.calls() << std::endl;
	assertEquals( "36 79 68 75 44 92 11 77 51 24 16 52 91 23 87 38 30 39 93 45 35 3 70 15 37 18"
				  " 19 73 42 72 80 12 78 84 96 76 89 31 43 50 82 81 85 98 22", turbo::str::join(correctorOne._corrected) );
	assertEquals( "", turbo::str::join(correctorTwo._corrected) );

	for (deque<string>::const_iterator it = correctorOne._corrected.begin(); it != correctorOne._corrected.end(); ++it)
		indexTwo.update(*it, 0);

	deque<string> filesOne = indexOne.find("fooid").enumerate(0,~0ULL,100);
	deque<string> filesTwo = indexTwo.find("fooid").enumerate(0,~0ULL,100);
	//assertEquals( turbo::str::join(filesOne), turbo::str::join(filesTwo) );
	assertEquals( 95, filesTwo.size() );
}

TEST_CASE( "SynchronizerExchangeTest/testCompareExchange.Case3", "[integration]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;

	MockLocateKeys locator;
	locator._locations.push_back("fooid");
	KeyTabulator indexOne(locator);
	KeyTabulator indexTwo(locator);

	indexOne.update(".membership/1", 1, 0);
	indexOne.update(".membership/2", 2, 0);
	indexOne.update(".membership/3", 3, 0);
	indexTwo.update(".membership/1", 1, 0);
	indexTwo.update(".membership/2", 2, 0);
	indexTwo.update(".membership/3", 3, 0);

	indexOne.update("foo", 4, 0);

	//static_cast<const DigestTree&>(indexOne.randomTree()).print(14);
	/*
	 * .membership/1: 0000 1010 | 1001 1001 | 1110 1000 | 1100 0100 | 0011 0001 | 0111 1110 | 0001 0101 | 1101 1110
	 *           foo: 0001 0000 | 0010 1000 | 1110 1100 | 0100 1111 | 0010 1000 | 1000 0010 | 0001 0010 | 1111 0111
	 * .membership/2: 0100 1101 | 0110 1001 | 0010 1001 | 0101 0001 | 0011 0101 | 1100 1100 | 1110 0000 | 0110 0111
	 * .membership/3: 1111 1011 | 0101 1111 | 0010 1001 | 1100 0000 | 0111 0101 | 1100 1100 | 0110 0000 | 0110 0100
	 */

	TestMessageSender senderOne;
	TestSkewCorrector correctorOne(indexOne);
	TestMessageSender senderTwo;
	TestSkewCorrector correctorTwo(indexTwo);

	//StderrLogger logger1("foo");
	//StderrLogger logger2("bar");
	MockLogger logger;
	Synchronizer one(ring, membership, indexOne, senderOne, correctorOne, logger);
	Synchronizer two(ring, membership, indexTwo, senderTwo, correctorTwo, logger);

	senderOne._other = &two;
	senderOne._corrector = &correctorTwo;
	senderTwo._other = &one;
	senderTwo._corrector = &correctorOne;

	// the compare is still too heavy-handed for my liking
	// 1. We ask for the whole subtree that's in disagreement ("/1" and "foo"), even though we already have "/1"
	// 2. Because the symmetric exchange hack, we end up double-healing in many cases, including this one. (why there are 4 instead of two entries in _corrected)
	one.compare(Peer("dummy"), TreeId("", 0), indexTwo.randomTree().top());
	assertEquals( "", turbo::str::join(correctorOne._healed) );
	assertEquals( ".membership/1 foo .membership/1 foo", turbo::str::join(correctorOne._corrected) );
	assertEquals( "", turbo::str::join(correctorTwo._healed) );
	assertEquals( "", turbo::str::join(correctorTwo._corrected) );
}
