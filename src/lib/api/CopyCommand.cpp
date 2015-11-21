/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "CopyCommand.h"

#include "WriteInstructions.h"

#include "storage/IStore.h"
#include "storage/readstream.h"
#include "storage/writestream.h"
#include <map>
#include <string>
using std::map;
using std::string;

CopyCommand::CopyCommand(IStore& store, std::function<void(WriteInstructions&, readstream&)> onCommit)
	: _store(store)
	, _onCommit(std::move(onCommit))
{
}

CopyCommand::~CopyCommand()
{
}

Turbopump::Request* CopyCommand::request()
{
	return &params;
}

bool CopyCommand::run(const char*, unsigned)
{
	if ( params.path.empty() )
		return setStatus(400);

	writestream writer = _store.write(params.name, params.version, params.copies, params.offset);
	readstream reader = writer.copy(params.path);
	if ( !reader )
		return setStatus(500);

	WriteInstructions instructions(params.name, params.version, params.mirror, params.copies);
	instructions.isComplete = true;
	_onCommit(instructions, reader);
	return true;
}
