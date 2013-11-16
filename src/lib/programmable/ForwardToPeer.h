#pragma once

#include "data_store/IDataStoreReader.h"
#include <string>

class IMembership;
class IPeerTracker;

class ForwardToPeer
{
public:
	ForwardToPeer(const IMembership& membership, IPeerTracker& peers);

	bool run(std::string filename, IDataStoreReader::ptr contents);

protected:
	const IMembership& _membership;
	IPeerTracker& _peers;
};
