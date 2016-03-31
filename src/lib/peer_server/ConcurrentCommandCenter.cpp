/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ConcurrentCommandCenter.h"

#include "PeerCommandRunner.h"
#include "api/Api.h"
#include "membership/Peer.h"

#include "concurrent/IExecutor.h"
using std::string;

ConcurrentCommandCenter::ConcurrentCommandCenter(Turbopump::Api& api, IExecutor& executor)
	: _api(api)
	, _executor(executor)
{
}

void ConcurrentCommandCenter::run(const std::shared_ptr<Peer>& peer, const std::string& buffer)
{
	string endpoint = peer->uid;
	std::shared_ptr<PeerCommandRunner>& runner = _runners[endpoint];
	if (!runner)
		runner.reset(new PeerCommandRunner(peer, *this));
	if (runner->addWork(buffer))
		_executor.execute(std::bind(&PeerCommandRunner::run, runner));
}

// may eventually want the markFinished() back.
// have two cleanup cases:
//  1. socket disconnects... can clean up...
//  2. once all inflight PeerCommands are finished.
// right now we don't clean up #2 at all. Also, should #1 wait for inflights to finish? Probably.
void ConcurrentCommandCenter::dismiss(const std::shared_ptr<Peer>& peer)
{
	_runners.erase(peer->uid);
}

std::shared_ptr<Turbopump::Command> ConcurrentCommandCenter::command(int cid, const char* buff, unsigned size)
{
	return _api.command(cid, buff, size);
}
