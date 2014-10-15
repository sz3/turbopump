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
class ILog;
class IMembership;
class ISocketWriter;
class Peer;

class PeerPacketHandler : public IPeerPacketHandler
{
public:
	PeerPacketHandler(Turbopump::Api& api, IExecutor& executor, const IMembership& membership, ILog& logger);

	bool onPacket(ISocketWriter& writer, const char* buff, unsigned size);

	std::shared_ptr<Turbopump::Command> command(unsigned cid, const char* buff, unsigned size);
	void markFinished(const std::string& id);

protected:
	Turbopump::Api& _api;
	IExecutor& _executor;
	const IMembership& _membership;
	ILog& _logger;

	std::unordered_map<std::string, std::shared_ptr<PeerCommandRunner>> _runners;
	tbb::concurrent_queue<std::string> _finished;
};


class PeerCommandRunner
{
public:
	PeerCommandRunner(const std::shared_ptr<Peer>& peer, IPeerPacketHandler& handler);

	void run();

	bool addWork(std::string&& buff);

protected:
	void doWorker();

public:
	std::shared_ptr<Peer> _peer;
	IPeerPacketHandler& _handler;

	std::atomic_flag _running = ATOMIC_FLAG_INIT;
	std::unordered_map<unsigned char,std::shared_ptr<Turbopump::Command>> _commands;
	tbb::concurrent_queue<std::string> _buffers;
};

