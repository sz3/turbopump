#pragma once

#include <atomic>
#include <memory>
#include "tbb/concurrent_queue.h"
class DataBuffer;
class IAction;
class IIpSocket;
class IpAddress;

// role of this class
// * to persist information about a conversation with a peer across multiple (small) packets (by holding the IAction)
// * (eventually) to encrypt outgoing communications?
// * to maintain a sequence number?

// TODO: will eventually need virtual connections to share physical fds.
// first pass is to restablish a connection everytime (since there's no NAT or encryption yet, this amounts to the client calling socket() again)
// and tie the command we want to run to the connection directly. It will move to the virtual once we gots one.
class PeerConnection
{
public:
	// should this be a weak ptr to the fd,
	// with the target peer split out into a separate data stucture?
	// for example, PeerSock{ IpAddress peer, std::weak_ptr<IIpSocket> }
	PeerConnection(const std::shared_ptr<IIpSocket>& sock);

	IpAddress peer() const;
	int send(const DataBuffer& data);

	bool begin_processing();
	void end_processing();

	void pushRecv(std::string buff);
	bool popRecv(std::string& buff);

	void setAction(const std::shared_ptr<IAction>& action);
	const std::shared_ptr<IAction>& action() const;

protected:
	std::atomic_flag _processing;
	tbb::concurrent_queue<std::string> _incoming;
	std::shared_ptr<IIpSocket> _sock;
	std::shared_ptr<IAction> _action;
};
