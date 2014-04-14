/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "LocalListAction.h"

#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "socket/StringByteStream.h"
#include <memory>
using std::map;
using std::string;

TEST_CASE( "LocalListActionTest/testAdd", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	LocalListAction action(store, stream);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "report(0,.membership/)", store._history.calls() );
}

TEST_CASE( "LocalListActionTest/testAdd.ShowAll", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	LocalListAction action(store, stream);

	map<string,string> params;
	params["all"] = "yes";
	params["deleted"] = "true";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "report(1,)", store._history.calls() );
}
