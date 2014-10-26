/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ConcurrentCommandCenter.h"
#include "api/DemandWrite.h"
#include "api/Write.h"
#include "membership/Peer.h"
#include "mock/DummyTurbopumpApi.h"
#include "peer_client/MessagePacker.h"

#include "event/SimpleExecutor.h"
#include <memory>
#include <string>
using std::string;

namespace {
	MessagePacker _packer;

	template <typename Req>
	string pack(Req req)
	{
		msgpack::sbuffer sbuf;
		msgpack::pack(&sbuf, req);
		string reqbuff = _packer.package(req._ID, sbuf.data(), sbuf.size());
		return _packer.package(33, reqbuff.data(), reqbuff.size());
	}

	string demand_write(std::string name)
	{
		Turbopump::DemandWrite req;
		req.name = name;
		return pack(req);
	}

	string write(std::string name)
	{
		Turbopump::Write req;
		req.name = name;
		return pack(req);
	}

	class TestableConcurrentCommandCenter : public ConcurrentCommandCenter
	{
	public:
		using ConcurrentCommandCenter::ConcurrentCommandCenter;
		using ConcurrentCommandCenter::_finished;
		using ConcurrentCommandCenter::_runners;
	};
}

TEST_CASE( "ConcurrentCommandCenterTest/testRun", "[unit]" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	TestableConcurrentCommandCenter center(api, executor);

	std::shared_ptr<Peer> peer(new Peer("hi"));
	center.run(peer, demand_write("foo"));

	assertEquals( 1, center._runners.size() );
	assertTrue( !!center._runners["hi"] );
	void* ptr = center._runners["hi"].get();
	assertEquals( "sendKey(hi,foo,,)", api.corrector._history.calls() );

	api.corrector._history.clear();
	center.run(peer, demand_write("foo"));

	assertEquals( 1, center._runners.size() );
	assertTrue( !!center._runners["hi"] );
	assertEquals( ptr, center._runners["hi"].get() );
	assertEquals( "sendKey(hi,foo,,)", api.corrector._history.calls() );
}

TEST_CASE( "ConcurrentCommandCenterTest/testRun.ClearFinished", "[unit]" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	TestableConcurrentCommandCenter center(api, executor);
	center._runners["finito"];
	assertEquals( 1, center._runners.size() );

	center.markFinished("finito");
	center.markFinished("hi");

	std::shared_ptr<Peer> peer(new Peer("hi"));
	center.run(peer, demand_write("foo"));

	assertEquals( 0, center._runners.size() );
	assertEquals( "sendKey(hi,foo,,)", api.corrector._history.calls() );
}

TEST_CASE( "ConcurrentCommandCenterTest/testMarkFinished", "[unit]" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	TestableConcurrentCommandCenter center(api, executor);

	center.markFinished("foo");
	center.markFinished("bar");
	assertEquals( 2, center._finished.unsafe_size() );
}

TEST_CASE( "ConcurrentCommandCenterTest/testCommand", "[unit]" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	TestableConcurrentCommandCenter center(api, executor);

	Turbopump::DemandWrite req;
	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, req);
	string reqbuff = _packer.package(req._ID, sbuf.data(), sbuf.size());

	std::shared_ptr<Turbopump::Command> cmd = center.command(req._ID, sbuf.data(), sbuf.size());
	assertNotNull( cmd.get() );
}

TEST_CASE( "ConcurrentCommandCenterTest/testRun.Multipacket", "[unit]" )
{
	DummyTurbopumpApi api;
	SimpleExecutor executor;
	TestableConcurrentCommandCenter center(api, executor);

	std::shared_ptr<Peer> peer(new Peer("hi"));
	center.run(peer, write("foo"));
	center.run(peer, _packer.package(33, "0123456789") + _packer.package(33, "bytes"));
	center.run(peer, _packer.package(35, "ignored") + _packer.package(33, "woohoo"));

	assertEquals( 1, center._runners.size() );
	assertEquals( 0, api.dataStore._store.size() );

	center.run(peer, _packer.package(33, "")); // flush
	assertEquals( 1, api.dataStore._store.size() );
	assertEquals( "0123456789byteswoohoo", api.dataStore._store["foo"] );

	assertEquals( 1, center._runners.size() );
}
