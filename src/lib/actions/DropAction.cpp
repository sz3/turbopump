/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DropAction.h"

#include "actions/DropParams.h"
#include "consistent_hashing/IHashRing.h"
#include "data_store/DataEntry.h"
#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"

#include "socket/IByteStream.h"
#include <algorithm>
using std::map;
using std::string;
using std::vector;

DropAction::DropAction(IDataStore& dataStore, const IHashRing& ring, const IMembership& membership, std::function<void(DropParams)> onDrop)
	: _dataStore(dataStore)
	, _ring(ring)
	, _membership(membership)
	, _onDrop(onDrop)
{
}

std::string DropAction::name() const
{
	return "drop";
}

bool DropAction::run(const DataBuffer& data)
{
	DropParams params;
	{
		IDataStoreReader::ptr read = _dataStore.read(_filename);
		if (!read)
			return false;

		vector<string> locs = _ring.locations(_filename, read->data().totalCopies);
		if (std::find(locs.begin(), locs.end(), _membership.self()->uid) != locs.end())
			return false;

		params.filename = _filename;
		params.totalCopies = read->data().totalCopies;
	}
	if (!_dataStore.erase(_filename))
		return false;

	if (_onDrop)
		_onDrop(std::move(params));
	return true;
}

void DropAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_filename = it->second;
}
