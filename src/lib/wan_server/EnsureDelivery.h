/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IBufferedConnectionWriter.h"

class EnsureDelivery
{
public:
	EnsureDelivery(bool ensureDelivery, IBufferedConnectionWriter& writer)
		: _ensureDelivery(ensureDelivery)
		, _writer(writer)
	{
		if (_ensureDelivery)
			_writer.ensureDelivery_inc();
	}

	~EnsureDelivery()
	{
		if (_ensureDelivery)
			_writer.ensureDelivery_dec();
	}

protected:
	bool _ensureDelivery;
	IBufferedConnectionWriter& _writer;
};
