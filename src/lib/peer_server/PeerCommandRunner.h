/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "concurrentqueue/concurrentqueue.h"
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
namespace Turbopump { class Command; }
class IPeerCommandCenter;
class Peer;

class PeerCommandRunner
{
public:
	PeerCommandRunner(const std::shared_ptr<Peer>& peer, IPeerCommandCenter& center);
	void shutdown();

	void run();
	void parseAndRun(const std::string& buffer);

	bool addWork(std::string buff);

protected:
	void doWork();

protected:
	std::shared_ptr<Peer> _peer;
	IPeerCommandCenter& _center;

	std::atomic_flag _running = ATOMIC_FLAG_INIT;
	std::unordered_map<unsigned char,std::shared_ptr<Turbopump::Command>> _commands;
	moodycamel::ConcurrentQueue<std::string> _buffers;
};


