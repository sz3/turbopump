/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "PeerCommandRunner.h"

#include "IPeerCommandCenter.h"
#include "PacketParser.h"
#include "api/Command.h"
#include "common/DataBuffer.h"
using std::string;

PeerCommandRunner::PeerCommandRunner(const std::shared_ptr<Peer>& peer, IPeerCommandCenter& center)
	: _peer(peer)
	, _center(center)
{
}

void PeerCommandRunner::shutdown()
{
	string sink;
	while (_buffers.try_dequeue(sink))
		;
}

void PeerCommandRunner::run()
{
	do {
		doWork();
		_running.clear();
	}
	while (_buffers.size_approx() > 0 && !_running.test_and_set());
}

void PeerCommandRunner::doWork()
{
	string buffer;
	while (_buffers.try_dequeue(buffer))
		parseAndRun(buffer);
}

void PeerCommandRunner::parseAndRun(const std::string& buffer)
{
	DataBuffer unparsed(buffer.data(), buffer.size());
	PacketParser packetGrabber(unparsed);

	DataBuffer buff(DataBuffer::Null());
	while (unparsed.size() > 0)
	{
		unsigned char virtid;
		if (!packetGrabber.getNext(virtid, buff))
			break;

		std::shared_ptr<Turbopump::Command> command;
		{
			auto it = _commands.find(virtid);
			if (it != _commands.end())
				command = it->second;
		}

		if (!command || command->finished())
		{
			PacketParser commandFinder(buff);
			unsigned char cid = 0;
			DataBuffer commandBuff(buff);
			if (!commandFinder.getNext(cid, commandBuff))
			{
				buff = commandBuff;
				continue;
			}

			command = _center.command(cid, commandBuff.buffer(), commandBuff.size());
			if (!command)
				continue;
			command->setPeer(_peer);

			if (!command->finished())
				_commands[virtid] = command;
		}
		command->run(buff.buffer(), buff.size());
	}
}

bool PeerCommandRunner::addWork(std::string buff)
{
	_buffers.enqueue(std::move(buff));
	return !_running.test_and_set();
}
