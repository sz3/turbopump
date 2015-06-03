/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class IKnowPeers;
class IMessageSender;
class WriteInstructions;
class readstream;

class NotifyWriteComplete
{
public:
	NotifyWriteComplete(const IKnowPeers& membership, IMessageSender& messenger);

	void run(WriteInstructions& params, readstream& contents);

protected:
	const IKnowPeers& _membership;
	IMessageSender& _messenger;
};
