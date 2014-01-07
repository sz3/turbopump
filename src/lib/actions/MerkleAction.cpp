/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MerkleAction.h"

#include "cohesion/ISynchronize.h"

#include "common/DataBuffer.h"
#include "common/MerklePoint.h"
#include "serialize/StringUtil.h"

using std::string;
using std::vector;

MerkleAction::MerkleAction(const Peer& peer, ISynchronize& sync)
	: _peer(peer)
	, _sync(sync)
{
}

std::string MerkleAction::name() const
{
	return "merkle";
}

/*
 * What we'll get:
 *   1) hash (assumed to be root hash)
 *   2) key + keybits + hash, either one or many? (at the minimum, either one or two)
 *
 * Could do this a couple ways:
 *   * rich data format (e.g. json), serializing some status in the DataBuffer
 *   * use the params field (key=value) and leave DataBuffer empty
 *   * params to specify encoding (e.g, number of hashes), with a serialized binary blob as the data
 *
 * binary encoding feels most attractive long term, but it may not make sense to do it now.
 */

bool MerkleAction::run(const DataBuffer& data)
{
	vector<string> points = StringUtil::split(data.str(), '|');
	for (vector<string>::const_iterator it = points.begin(); it != points.end(); ++it)
	{
		MerklePoint point;
		if (!MerklePointSerializer::fromString(point, *it))
			return false;

		_sync.compare(_peer, point);
	}
	return true;
}

void MerkleAction::setParams(const std::map<std::string,std::string>& params)
{
}
