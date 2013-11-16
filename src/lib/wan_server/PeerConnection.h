#pragma once

#include <memory>
class DataBuffer;
class IAction;
class IIpSocket;
class IpAddress;

// role of this class
// * to persist information about a conversation with a peer across multiple (small) packets

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
	int write(const DataBuffer& data);

	void setAction(const std::shared_ptr<IAction>& action);
	const std::shared_ptr<IAction>& action() const;

protected:
	std::shared_ptr<IIpSocket> _sock;
	std::shared_ptr<IAction> _action;
};
