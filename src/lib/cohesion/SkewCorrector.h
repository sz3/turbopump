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
	void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

protected:
	const IMerkleIndex& _index;
	const IDataStore& _store;
	IWriteActionSender& _sender;
};
