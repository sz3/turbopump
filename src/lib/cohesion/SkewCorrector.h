/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ICorrectSkew.h"

class IDataStore;
class IMerkleIndex;
class IWriteActionSender;

class SkewCorrector : public ICorrectSkew
{
public:
	SkewCorrector(const IMerkleIndex& index, const IDataStore& store, IWriteActionSender& sender);

	void healKey(const Peer& peer, unsigned long long key);
	void pushKeyRange(const Peer& peer, const std::string& treeid, unsigned long long first, unsigned long long last);

protected:
	const IMerkleIndex& _index;
	const IDataStore& _store;
	IWriteActionSender& _sender;
};
