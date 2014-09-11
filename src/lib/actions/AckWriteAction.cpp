/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AckWriteAction.h"

#include "actions/DropParams.h"
#include "common/KeyMetadata.h"
#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "hashing/ILocateKeys.h"
using std::map;
using std::string;

AckWriteAction::AckWriteAction(IDataStore& store, const ILocateKeys& locator, std::function<void(DropParams)> onDrop)
	: _store(store)
	, _locator(locator)
	, _onDrop(onDrop)
{
}

std::string AckWriteAction::name() const
{
	return "ack-write";
}

bool AckWriteAction::run(const DataBuffer& data)
{
	IDataStoreReader::ptr reader = _store.read(_filename, _version);
	if (!reader)
		return false;

	if (reader->size() != _size)
		return false;

	unsigned short totalCopies = reader->metadata().totalCopies;
	if (!_locator.keyIsMine(_filename, totalCopies))
	{
		DropParams params;
		params.filename = _filename;
		params.totalCopies = totalCopies;
		drop(params);
	}
	return true;
}

bool AckWriteAction::drop(const DropParams& params)
{
	if (!_store.drop(params.filename))
		return false;
	if (_onDrop)
		_onDrop(params);
	return true;
}

void AckWriteAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_filename = it->second;

	it = params.find("v");
	if (it != params.end())
		_version = it->second;

	it = params.find("size");
	if (it != params.end())
		_size = std::stoull(it->second);
}
