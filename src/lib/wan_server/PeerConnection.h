/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include "tbb/concurrent_queue.h"
class IAction;
class VirtualConnection;

// role of this class
// * to persist information about a conversation with a peer across multiple (small) server packets (by holding the IAction)
// * to buffer incoming packets from a single peer, so they can be run in order on a separate thread (freeing up the udp thread for better things)

// TODO: will eventually need virtual connections to share physical fds.
// first pass is to restablish a connection everytime (since there's no NAT or encryption yet, this amounts to the client calling socket() again)
// and tie the command we want to run to the connection directly. It will move to the virtual once we gots one.
class PeerConnection
{
public:
	bool begin_processing();
	void end_processing();

	void pushRecv(std::string buffer);
	bool popRecv(std::string& buffer);
	bool empty() const;

	VirtualConnection& operator[](unsigned char vid);
	VirtualConnection& virt(unsigned char vid);
	std::shared_ptr<IAction> action(unsigned char vid);

protected:
	std::atomic_flag _processing;
	tbb::concurrent_queue<std::string> _incoming;
	std::array<std::shared_ptr<VirtualConnection>,256> _virts;
};
