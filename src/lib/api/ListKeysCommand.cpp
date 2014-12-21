/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ListKeysCommand.h"

#include "common/turbopump_defaults.h"
#include "storage/IStore.h"

#include "socket/IByteStream.h"
#include <functional>
using std::string;
using namespace std::placeholders;

ListKeysCommand::ListKeysCommand(const IStore& store)
	: _store(store)
{
}

bool ListKeysCommand::print_key(const std::string& report) const
{
	if (!params.all && report.find(MEMBERSHIP_FILE_PREFIX) == 0)
		return true;
	if (!params.deleted)
	{
		size_t first_clock = report.find('|');
		if (first_clock == string::npos)
			return true;
		first_clock = report.find(',', first_clock);
		if (first_clock == string::npos)
			return true;
		if (report.find(",deleted:", first_clock) == first_clock)
			return true;
	}

	string data = report + "\n";
	_stream->write(data.data(), data.size());
	return true;
}

bool ListKeysCommand::run(const char*, unsigned)
{
	if (!_stream)
		return false;

	_store.enumerate(std::bind(&ListKeysCommand::print_key, this, _1), 1000);
	return true;
}

Turbopump::Request* ListKeysCommand::request()
{
	return &params;
}
