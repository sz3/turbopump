/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"
class IMembership;
class IMessageSender;
class WriteParams;

class NotifyWriteComplete
{
public:
	NotifyWriteComplete(const IMembership& membership, IMessageSender& messenger);

	void run(WriteParams& params, IDataStoreReader::ptr contents);

protected:
	const IMembership& _membership;
	IMessageSender& _messenger;
};
