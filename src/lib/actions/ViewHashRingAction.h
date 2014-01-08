/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "MessageAction.h"

class IHashRing;

class ViewHashRingAction : public MessageAction
{
public:
	ViewHashRingAction(const IHashRing& ring, IByteStream& writer);
};
