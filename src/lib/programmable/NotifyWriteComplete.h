/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class IMembership;
class IMessageSender;
class WriteInstructions;
class readstream;

class NotifyWriteComplete
{
public:
	NotifyWriteComplete(const IMembership& membership, IMessageSender& messenger);

	void run(WriteInstructions& params, readstream& contents);

protected:
	const IMembership& _membership;
	IMessageSender& _messenger;
};
