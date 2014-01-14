/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboCluster.h"

#include "TurboRunner.h"
#include "serialize/StringUtil.h"

TurboCluster::TurboCluster(unsigned workers, std::string programFlags)
{
	TurboRunner::createMemberFile(workers);
	for (unsigned i = 1; i <= workers; ++i)
		_workers.push_back( TurboRunner(9000+i, "/tmp/turbo" + StringUtil::str(9000+i), programFlags) );
}

void TurboCluster::start()
{
	for (TurboRunner& runner : _workers)
		runner.start();
}

bool TurboCluster::waitForRunning(unsigned seconds)
{
	for (TurboRunner& runner : _workers)
		if (!runner.waitForRunning(seconds))
			return setError(runner.dataChannel() + " timed out waiting for running");
	return true;
}

// start counting at 1! ... since that's the worker id anyway...
TurboRunner& TurboCluster::operator[](unsigned num)
{
	--num;
	if (num >= _workers.size())
		num = _workers.size()-1;
	return _workers[num];
}

const std::string& TurboCluster::lastError() const
{
	return _lastError;
}

bool TurboCluster::setError(const std::string& msg)
{
	_lastError = msg;
	return false;
}
