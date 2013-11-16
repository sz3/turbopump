#pragma once

#include "socket/IByteStream.h"
#include <memory>
class PeerConnection;

// using PeerConnection as the throttling mechanism. If we've received an appropriate amount of acks, keep going.
// if not, wait until we have.
// needs handle to some kind of threadpool, to schedule work when he's allowed to run again.
class ThrottledWriteStream : public IByteStream
{
public:
	// should this be held by the PeerConn, or vice versa?
	ThrottledWriteStream(PeerConnection& conn);

	unsigned maxPacketLength() const;

	int read(char* buffer, unsigned length);
	int write(const char* buffer, unsigned length);

protected:
	PeerConnection& _conn;
};

