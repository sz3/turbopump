/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SkewCorrector.h"

#include "TreeId.h"
#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockKeyTabulator.h"
#include "mock/MockLogger.h"
#include "mock/MockWriteActionSender.h"

using std::string;

TEST_CASE( "SkewCorrectorTest/testPushKeyRange", "[unit]" )
{
	MockKeyTabulator index;
	MockDataStore store;
	MockWriteActionSender writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, writer, logger);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("badfile");
	index._tree._enumerate.push_back("file3");
	store._store["file1"] = "I am file 1";
	store._store["file3"] = "I am file 3";

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak",2), 0, 1234567890);

	assertEquals( "find(oak,2)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "store(fooid,file1,1,1,[1,mockReaderVersion:1],)"
				  "|store(fooid,file3,1,1,[1,mockReaderVersion:1],)", writer._history.calls() );
	assertEquals( "read(file1)|read(badfile)|read(file3)", store._history.calls() );
	assertEquals( "logDebug(pushing 3 keys to peer fooid: file1 badfile file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Offload", "[unit]" )
{
	MockKeyTabulator index;
	MockDataStore store;
	MockWriteActionSender writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, writer, logger);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("badfile");
	index._tree._enumerate.push_back("file3");
	store._store["file1"] = "I am file 1";
	store._store["file3"] = "I am file 3";

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890, "offloadFrom");

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "store(fooid,file1,1,1,[1,mockReaderVersion:1],offloadFrom)"
				  "|store(fooid,file3,1,1,[1,mockReaderVersion:1],offloadFrom)", writer._history.calls() );
	assertEquals( "read(file1)|read(badfile)|read(file3)", store._history.calls() );
	assertEquals( "logDebug(pushing 3 keys to peer fooid: file1 badfile file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Empty", "[unit]" )
{
	MockKeyTabulator index;
	MockDataStore store;
	MockWriteActionSender writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, writer, logger);

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890);

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", store._history.calls() );
	assertEquals( "logDebug(pushing 0 keys to peer fooid: )", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.ConnectionExplodes", "[unit]" )
{
	MockKeyTabulator index;
	MockDataStore store;
	MockWriteActionSender writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, writer, logger);

	writer._storeFails = true;

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("badfile");
	index._tree._enumerate.push_back("file3");
	store._store["file1"] = "I am file 1";
	store._store["file3"]= "I am file 3";

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890);

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "store(fooid,file1,1,1,[1,mockReaderVersion:1],)", writer._history.calls() );
	assertEquals( "read(file1)", store._history.calls() );
	assertEquals( "logDebug(pushing 3 keys to peer fooid: file1 badfile file3)", logger._history.calls() );
}
