/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SkewCorrector.h"

#include "TreeId.h"
#include "membership/Peer.h"
#include "mock/MockKeyTabulator.h"
#include "mock/MockLogger.h"
#include "mock/MockMessageSender.h"
#include "mock/MockStore.h"
#include "mock/MockWriteSupervisor.h"

using std::string;

TEST_CASE( "SkewCorrectorTest/testHealKey", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("file2");
	store._versions.push_back("v1");
	store._versions.push_back("v2");

	corrector.healKey(Peer("fooid"), TreeId("oak",2), 12345678);

	assertEquals( "find(oak,2)", index._history.calls() );
	assertEquals( "enumerate(12345678,12345678)", index._tree._history.calls() );
	assertEquals( "offerWrite(fooid,file1,v1,)|offerWrite(fooid,file1,v2,)"
				  "|offerWrite(fooid,file2,v1,)|offerWrite(fooid,file2,v2,)", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "versions(file1,0)|versions(file2,0)", store._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testHealKey.Nothing", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	corrector.healKey(Peer("fooid"), TreeId("oak",2), 12345678);

	assertEquals( "find(oak,2)", index._history.calls() );
	assertEquals( "enumerate(12345678,12345678)", index._tree._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", store._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("badfile");
	index._tree._enumerate.push_back("file3");
	store._reads["file1"] = "I am file 1";
	store._reads["file3"] = "I am file 3";
	store._versions.push_back("1,version:1");

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak",2), 0, 1234567890);

	assertEquals( "find(oak,2)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "store(fooid,file1,3,3,[1,version:1],,1)"
				  "|store(fooid,file3,3,3,[1,version:1],,1)", writer._history.calls() );
	assertEquals( "readAll(file1)|readAll(badfile)|readAll(file3)", store._history.calls() );
	assertEquals( "logDebug(pushing 3 keys to peer fooid: file1 badfile file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Offload", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("badfile");
	index._tree._enumerate.push_back("file3");
	store._reads["file1"] = "I am file 1";
	store._reads["file3"] = "I am file 3";
	store._versions.push_back("1,version:1");

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890, "offloadFrom");

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "store(fooid,file1,3,3,[1,version:1],offloadFrom,1)"
				  "|store(fooid,file3,3,3,[1,version:1],offloadFrom,1)", writer._history.calls() );
	assertEquals( "readAll(file1)|readAll(badfile)|readAll(file3)", store._history.calls() );
	assertEquals( "logDebug(pushing 3 keys to peer fooid: file1 badfile file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Empty", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890);

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", store._history.calls() );
	assertEquals( "logDebug(pushing 0 keys to peer fooid: )", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.ConnectionExplodes", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	writer._storeFails = true;

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("badfile");
	index._tree._enumerate.push_back("file3");
	store._reads["file1"] = "I am file 1";
	store._reads["file3"] = "I am file 3";
	store._versions.push_back("1,version:1");

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890);

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "store(fooid,file1,3,3,[1,version:1],,1)", writer._history.calls() );
	assertEquals( "readAll(file1)", store._history.calls() );
	assertEquals( "logDebug(pushing 3 keys to peer fooid: file1 badfile file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testSendKey", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	store._reads["file1"] = "I am file 1";

	assertTrue( corrector.sendKey(Peer("fooid"), "file1", "v1", "sauce") );

	assertEquals( "read(file1,v1)", store._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "store(fooid,file1,3,3,[v1],sauce,1)", writer._history.calls() );
	assertEquals( "", logger._history.calls() );
	assertEquals( "", index._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testSendKey.Empty", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	assertFalse( corrector.sendKey(Peer("fooid"), "no file", "v1", "sauce") );

	assertEquals( "read(no file,v1)", store._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", logger._history.calls() );
	assertEquals( "", index._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testSendKey.ConnectionExplodes", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	SkewCorrector corrector(index, store, messenger, writer, logger);

	writer._storeFails = true;
	store._reads["file1"] = "I am file 1";

	assertFalse( corrector.sendKey(Peer("fooid"), "file1", "v1", "sauce") );

	assertEquals( "read(file1,v1)", store._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "store(fooid,file1,3,3,[v1],sauce,1)", writer._history.calls() );
	assertEquals( "logError(sendKey failed to store file [file1,v1] to peer fooid)", logger._history.calls() );
	assertEquals( "", index._history.calls() );
}

