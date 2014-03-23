/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SyncAction.h"

#include "cohesion/ISynchronize.h"
#include "common/DataBuffer.h"
#include "common/MerklePoint.h"
#include "serialize/StringUtil.h"
using std::map;
using std::string;
using std::vector;

SyncAction::SyncAction(const Peer& peer, ISynchronize& sync)
	: _peer(peer)
	, _sync(sync)
{
}

std::string SyncAction::name() const
{
	return "sync";
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

bool SyncAction::run(const DataBuffer& data)
{
	vector<string> points = StringUtil::split(data.str(), '|');
	for (unsigned i = 0; i < points.size(); ++i)
	{
		MerklePoint point;
		if (!MerklePointSerializer::fromString(point, points[i]))
			return false;

		_sync.compare(_peer, _tree, point, i >= 2);
	}
	return true;
}

void SyncAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("tree");
	if (it != params.end())
		_tree.id = it->second;

	it = params.find("n");
	if (it != params.end())
		_tree.mirrors = std::stoul(it->second);
}
