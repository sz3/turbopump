#include "unittest.h"

#include "KeyReqAction.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockSynchronize.h"
using std::map;
using std::string;

TEST_CASE( "KeyReqActionTest/testDefault", "[unit]" )
{
	Peer peer("fooid");
	MockSynchronize sync;
	KeyReqAction action(peer, sync);

	map<string,string> params;
	params["first"] = "52";
	params["last"] = "1337";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );
	assertEquals( "pushKeyRange(fooid,52,1337)", sync._history.calls() );
}
