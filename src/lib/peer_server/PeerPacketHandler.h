/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IPeerPacketHandler.h"
#include "tbb/concurrent_queue.h"
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
namespace Turbopump { class Api; class Command; }
class IExecutor;
class IMembership;
class ISocketWriter;
class Peer;

class PeerPacketHandler : public IPeerPacketHandler
{
public:
	PeerPacketHandler(Turbopump::Api& api, const IMembership& membership, IPeerCommandCenter& center);

	bool onPacket(ISocketWriter& writer, const char* buff, unsigned size);

protected:
	Turbopump::Api& _api;
	const IMembership& _membership;
	IPeerCommandCenter& _center;
};

class PeerCommandCenter : public IPeerCommandCenter
{
public:
	PeerCommandCenter(IExecutor& executor);

	void run(const std::shared_ptr<Peer>& peer, const std::string& buffer);
	void markFinished(const std::string& id);

protected:

	IExecutor& _executor;

	std::unordered_map<std::string, std::shared_ptr<PeerCommandRunner>> _runners;
	tbb::concurrent_queue<std::string> _finished;
};


// maybe split into a subclass that just handles the simple stuff?
class PeerCommandRunner
{
public:
	PeerCommandRunner(const std::shared_ptr<Peer>& peer, IPeerCommandCenter& center);

	void run();
	void parseAndRun(const std::string& buffer);

	bool addWork(std::string&& buff);

protected:
	void doWork();

public:
	std::shared_ptr<Peer> _peer;
	IPeerCommandCenter& _center;

	std::atomic_flag _running = ATOMIC_FLAG_INIT;
	std::unordered_map<unsigned char,std::shared_ptr<Turbopump::Command>> _commands;
	tbb::concurrent_queue<std::string> _buffers;
};

