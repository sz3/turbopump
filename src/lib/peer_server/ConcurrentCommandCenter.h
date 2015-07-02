/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IPeerCommandCenter.h"
#include "concurrentqueue/concurrentqueue.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace Turbopump { class Api; }
class IExecutor;
class Peer;
class PeerCommandRunner;

class ConcurrentCommandCenter : public IPeerCommandCenter
{
public:
	ConcurrentCommandCenter(Turbopump::Api& api, IExecutor& executor);

	void run(const std::shared_ptr<Peer>& peer, const std::string& buffer);
	void markFinished(const std::string& id);

	std::shared_ptr<Turbopump::Command> command(int cid, const char* buff, unsigned size);

protected:
	Turbopump::Api& _api;
	IExecutor& _executor;

	std::unordered_map<std::string, std::shared_ptr<PeerCommandRunner>> _runners;
	moodycamel::ConcurrentQueue<std::string> _finished;
};

