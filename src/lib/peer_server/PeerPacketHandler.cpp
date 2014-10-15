/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "PeerPacketHandler.h"

#include "api/Api.h"
#include "event/IExecutor.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include "socket/ISocketWriter.h"
#include "socket_address.h"
using std::shared_ptr;
using std::string;

PeerPacketHandler::PeerPacketHandler(Turbopump::Api& api, const IMembership& membership, IPeerCommandCenter& center)
	: _api(api)
	, _membership(membership)
	, _center(center)
{
}

bool PeerPacketHandler::onPacket(ISocketWriter& writer, const char* buff, unsigned size)
{
	// is the message from a valid peer?
	std::shared_ptr<Peer> peer = _membership.lookupIp(writer.target());
	if (!peer)
	{
		_logger.logWarn("rejecting packet from unknown host " + writer.endpoint().toString());
		return false;
	}

	if (size == 0)
		return false;
	string buffer(buff, size); // decrypt here

	_center.run(peer, buffer);
	return true;
}

std::shared_ptr<Turbopump::Command> PeerPacketHandler::command(unsigned cid, const char* buff, unsigned size)
{
	return _api.command(cid, buff, size);
}


PeerCommandCenter::PeerCommandCenter(IExecutor& executor)
	: _executor(executor)
{
}

void PeerCommandCenter::run(const std::shared_ptr<Peer>& peer, const std::string& buffer)
{
	string endpoint = peer->uid;
	shared_ptr<PeerCommandRunner> runner =_runners[endpoint];
	if (!runner)
		runner.set(new PeerCommandRunner(peer));
	if (runner->addWork(std::move(buffer)))
		_executor.execute(std::bind(&PeerCommandRunner::run, runner));

	string fin;
	while (_finished.try_pop(fin))
		_runners.erase(fin);
}

void PeerCommandCenter::markFinished(const std::string& runner)
{
	_finished.push(runner);
}

//PeerCommandRunner

PeerCommandRunner::PeerCommandRunner(const std::shared_ptr<Peer>& peer, IPeerCommandCenter& center)
	: _peer(peer)
	, _center(center)
{
}

void PeerCommandRunner::run()
{
	do {
		doWork();
		_running.clear();
	}
	while (!_buffers.empty() && !_running.test_and_set());
}

void PeerCommandRunner::doWork()
{
	string buffer;
	while (_buffers.try_pop(buffer))
		parseAndRun(buffer);
	// if no more actions and no more buffers, _center.markFinished(_peer->uid)
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

		std::shared_ptr<Turbopump::Command> command = _commands[virtid];
		if (!command || command->finished())
		{
			PacketParser commandFinder(buff);
			unsigned char cid = 0;
			DataBuffer commandBuff(buff);
			if (!commandFinder.getNext(cid, commandBuff))
				break;

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


bool PeerCommandRunner::addWork(std::string&& buff)
{
	_buffers.push(buff);
	return !_running.test_and_set();
}
