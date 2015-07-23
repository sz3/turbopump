/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "SkewCorrector.h"

#include "TreeId.h"
#include "api/Drop.h"
#include "api/Options.h"
#include "membership/Peer.h"
#include "mock/MockKeyTabulator.h"
#include "mock/MockLogger.h"
#include "mock/MockMessageSender.h"
#include "mock/MockStore.h"
#include "mock/MockWriteSupervisor.h"

using std::string;

TEST_CASE( "SkewCorrectorTest/testDropKey", "[unit]" )
{
	CallHistory history;

	MockKeyTabulator index;
	MockStore store;
	store._reads["dropme"] = "foo";
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;

	opts.when_drop_finishes = [&](const Turbopump::Drop& md){ history.call("onDrop", md.name, md.copies); };
	opts.when_drop_finishes.finalize();
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	assertTrue( corrector.dropKey("dropme") );
	assertEquals( "onDrop(dropme,3)", history.calls() );

	assertEquals( "read(dropme,)|remove(dropme)", store._history.calls() );
	assertEquals( "", index._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testDropKey.NoFile", "[unit]" )
{
	CallHistory history;

	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;

	opts.when_drop_finishes = [&](const Turbopump::Drop& md){ history.call("onDrop", md.name, md.copies); };
	opts.when_drop_finishes.finalize();
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	assertFalse( corrector.dropKey("dropme") );
	assertEquals( "", history.calls() );

	assertEquals( "read(dropme,)", store._history.calls() );
	assertEquals( "", index._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKey", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("file2");
	store._versions.push_back("v1");
	store._versions.push_back("v2");

	corrector.pushKey(Peer("fooid"), TreeId("oak",2), 12345678);

	assertEquals( "find(oak,2)", index._history.calls() );
	assertEquals( "enumerate(12345678,12345678)", index._tree._history.calls() );
	assertEquals( "offerWrite(fooid,file1,v1,)|offerWrite(fooid,file1,v2,)"
				  "|offerWrite(fooid,file2,v1,)|offerWrite(fooid,file2,v2,)", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "versions(file1,1)|versions(file2,1)", store._history.calls() );
	assertEquals( "logDebug(pushing 2 keys to peer fooid: file1 file2)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKey.Nothing", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	corrector.pushKey(Peer("fooid"), TreeId("oak",2), 12345678);

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
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("file3");
	store._reads["file1"] = "I am file 1";
	store._reads["file3"] = "I am file 3";
	store._versions.push_back("1,version.UNIXSECONDS=");

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak",2), 0, 1234567890);

	assertEquals( "find(oak,2)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "offerWrite(fooid,file1,1,version.UNIXSECONDS=,)"
				  "|offerWrite(fooid,file3,1,version.UNIXSECONDS=,)", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "versions(file1,1)|isExpired(1,version.UNIXSECONDS=)"
				  "|versions(file3,1)|isExpired(1,version.UNIXSECONDS=)", store._history.calls() );
	assertEquals( "logDebug(pushing 2 keys to peer fooid: file1 file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Offload", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	index._tree._enumerate.push_back("file1");
	index._tree._enumerate.push_back("file3");
	store._reads["file1"] = "I am file 1";
	store._reads["file3"] = "I am file 3";
	store._versions.push_back("1,version.UNIXSECONDS=");

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890, "offloadFrom");

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "offerWrite(fooid,file1,1,version.UNIXSECONDS=,offloadFrom)"
				  "|offerWrite(fooid,file3,1,version.UNIXSECONDS=,offloadFrom)", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "versions(file1,1)|isExpired(1,version.UNIXSECONDS=)"
				  "|versions(file3,1)|isExpired(1,version.UNIXSECONDS=)", store._history.calls() );
	assertEquals( "logDebug(pushing 2 keys to peer fooid: file1 file3)", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testPushKeyRange.Empty", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	corrector.pushKeyRange(Peer("fooid"), TreeId("oak"), 0, 1234567890);

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "enumerate(0,1234567890)", index._tree._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer._history.calls() );
	assertEquals( "", store._history.calls() );
	assertEquals( "", logger._history.calls() );
}

TEST_CASE( "SkewCorrectorTest/testSendKey", "[unit]" )
{
	MockKeyTabulator index;
	MockStore store;
	MockMessageSender messenger;
	MockWriteSupervisor writer;
	MockLogger logger;
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

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
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

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
	Turbopump::Options opts;
	SkewCorrector corrector(index, store, messenger, writer, logger, opts);

	writer._storeFails = true;
	store._reads["file1"] = "I am file 1";

	assertFalse( corrector.sendKey(Peer("fooid"), "file1", "v1", "sauce") );

	assertEquals( "read(file1,v1)", store._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "store(fooid,file1,3,3,[v1],sauce,1)", writer._history.calls() );
	assertEquals( "logError(sendKey failed to store file [file1,v1] to peer fooid)", logger._history.calls() );
	assertEquals( "", index._history.calls() );
}

