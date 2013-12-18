#include "PeerConnection.h"

#include "VirtualConnection.h"

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

void PeerConnection::pushRecv(OrderedPacket packet)
{
	_incoming.push(packet);
}

bool PeerConnection::popRecv(OrderedPacket& packet)
{
	return _incoming.try_pop(packet);
}

bool PeerConnection::empty() const
{
	return _incoming.empty();
}

VirtualConnection& PeerConnection::operator[](unsigned char vid)
{
	return virt(vid);
}

VirtualConnection& PeerConnection::virt(unsigned char vid)
{
	std::shared_ptr<VirtualConnection>& vii = _virts[vid];
	if (!vii)
		vii.reset(new VirtualConnection);
	return *vii;
}

std::shared_ptr<IAction> PeerConnection::action(unsigned char vid)
{
	std::shared_ptr<VirtualConnection>& vii = _virts[vid];
	if (!vii)
		return NULL;
	return vii->action();
}
