/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class TurboRunner;
#include <deque>
#include <string>

class TurboCluster
{
public:
	TurboCluster(unsigned workers, std::string programFlags="");

	void start();
	bool waitForRunning(unsigned seconds=5);

	TurboRunner& operator[](unsigned num);

	const std::string& lastError() const;

protected:
	bool setError(const std::string& msg);

protected:
	std::string _lastError;
	std::deque<TurboRunner> _workers;
};
