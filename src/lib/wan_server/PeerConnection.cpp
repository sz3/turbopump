#include "PeerConnection.h"

// manage buffers.
/*
 * udp server -> pushes buffer (concurrent_queue) into connection
 * udp server -> atomic notify (check lock, if thread is running it should be held). If thread is not running, schedule work
 * running thread -> reads next buffer
 *
 * ... in this way, we can keep work for a given operation on the same thread.
 */

// TODO: expiry! Seriously, how?
// TODO: virtual connection map for actions

bool PeerConnection::begin_processing()
{
	return !_incoming.empty() && !_processing.test_and_set();
}

void PeerConnection::end_processing()
{
	_processing.clear();
}

void PeerConnection::pushRecv(std::string buff)
{
	_incoming.push(std::move(buff));
}

bool PeerConnection::popRecv(std::string& buff)
{
	return _incoming.try_pop(buff);
}

void PeerConnection::setAction(const std::shared_ptr<IAction>& action)
{
	_action = action;
}

const std::shared_ptr<IAction>& PeerConnection::action() const
{
	return _action;
}
