#include "MockHashRing.h"

void MockHashRing::addWorker(const std::string& id)
{
	_history.call("addWorker", id);
}

void MockHashRing::removeWorker(const std::string& id)
{
	_history.call("removeWorker", id);
}

std::vector<std::string> MockHashRing::lookup(const std::string& filename, unsigned numWorkers) const
{
	_history.call("lookup", filename, numWorkers);
	return _workers;
}
