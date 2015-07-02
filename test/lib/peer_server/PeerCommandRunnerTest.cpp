/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "PeerCommandRunner.h"
#include "api/DemandWrite.h"
#include "membership/Peer.h"
#include "mock/MockCommand.h"
#include "mock/MockPeerCommandCenter.h"
#include "peer_client/MessagePacker.h"
using std::string;

namespace {
	MessagePacker _packer;

	string commandEnc(unsigned char commandId, std::string data)
	{
		// total dummy, since we don't have to care about real parsing.
		return _packer.package(commandId, data);
	}

	class TestablePeerCommandRunner : public PeerCommandRunner
	{
	public:
		using PeerCommandRunner::PeerCommandRunner;
		using PeerCommandRunner::_buffers;
		using PeerCommandRunner::_commands;
		using PeerCommandRunner::_running;
	};
}

TEST_CASE( "PeerCommandRunnerTest/testAddWork", "[unit]" )
{
	std::shared_ptr<Peer> peer(new Peer("foo"));
	MockPeerCommandCenter center;
	TestablePeerCommandRunner runner(peer, center);

	assertTrue( runner.addWork("woohoo") );
	assertFalse( runner.addWork("yippee") );
	assertEquals( 2, runner._buffers.size_approx() );

	runner._running.clear();
	assertTrue( runner.addWork("yee haw!") );
	assertEquals( 3, runner._buffers.size_approx() );
}

TEST_CASE( "PeerCommandRunnerTest/testRun", "[unit]" )
{
	std::shared_ptr<Peer> peer(new Peer("foo"));
	MockPeerCommandCenter center;
	center._commands[84].reset(new MockCommand);
	center._commands[86].reset(new MockCommand);
	center._commands[87].reset(new MockCommand);
	TestablePeerCommandRunner runner(peer, center);

	assertTrue( runner.addWork(_packer.package(33, commandEnc(87, "brooks"))) );
	runner.addWork( _packer.package(33, "kaboom") + _packer.package(33, commandEnc(84, "sharpe")) );
	runner.addWork( _packer.package(35, commandEnc(86, "freeman")) );
	assertEquals( 3, runner._buffers.size_approx() );

	runner.run();

	assertEquals( 0, runner._buffers.size_approx() );
	assertEquals( 0, runner._commands.size() );
	assertEquals( "command(87,brooks)|command(84,sharpe)|command(86,freeman)", center._history.calls() );
	assertEquals( "run()", (static_cast<MockCommand*>(center._commands[84].get()))->_history.calls() );
	assertEquals( "run()", (static_cast<MockCommand*>(center._commands[86].get()))->_history.calls() );
	assertEquals( "run()", (static_cast<MockCommand*>(center._commands[87].get()))->_history.calls() );
}

TEST_CASE( "PeerCommandRunnerTest/testRun.BadCommand", "[unit]" )
{
	std::shared_ptr<Peer> peer(new Peer("foo"));
	MockPeerCommandCenter center;
	TestablePeerCommandRunner runner(peer, center);

	assertTrue( runner.addWork(_packer.package(37, commandEnc(85, "jennings"))) );
	runner.run();

	assertEquals( 0, runner._buffers.size_approx() );
	assertEquals( 0, runner._commands.size() );
	assertEquals( "command(85,jennings)", center._history.calls() );
}

TEST_CASE( "PeerCommandRunnerTest/testRun.BadParse", "[unit]" )
{
	std::shared_ptr<Peer> peer(new Peer("foo"));
	MockPeerCommandCenter center;
	TestablePeerCommandRunner runner(peer, center);

	assertTrue( runner.addWork(_packer.package(37, "kaboom")) );
	runner.run();

	assertEquals( 0, runner._buffers.size_approx() );
	assertEquals( 0, runner._commands.size() );
	assertEquals( "", center._history.calls() );
}

TEST_CASE( "PeerCommandRunnerTest/testRun.Unfinished", "[unit]" )
{
	std::shared_ptr<Peer> peer(new Peer("foo"));
	MockPeerCommandCenter center;
	center._commands[100].reset(new MockCommand(false));
	center._commands[101].reset(new MockCommand(false));
	center._commands[102].reset(new MockCommand(false));
	TestablePeerCommandRunner runner(peer, center);

	assertTrue( runner.addWork(_packer.package(33, commandEnc(100, "header"))) );
	runner.addWork( _packer.package(33, "data") + _packer.package(35, commandEnc(101, "diffheader")) + _packer.package(33, "moredata") );
	runner.addWork( _packer.package(37, "kaboom") + _packer.package(34, commandEnc(102, "I'm new here")) );
	runner.addWork( _packer.package(37, "kaboom") + _packer.package(35, "other data") );

	runner.run();

	assertEquals( 0, runner._buffers.size_approx() );
	assertEquals( 3, runner._commands.size() );
	assertEquals( "command(100,header)|command(101,diffheader)|command(102,I'm new here)", center._history.calls() );
	assertEquals( "run()|run(data)|run(moredata)", (static_cast<MockCommand*>(center._commands[100].get()))->_history.calls() );
	assertEquals( "run()|run(other data)", (static_cast<MockCommand*>(center._commands[101].get()))->_history.calls() );
	assertEquals( "run()", (static_cast<MockCommand*>(center._commands[102].get()))->_history.calls() );

	// TODO: clean up.
}

