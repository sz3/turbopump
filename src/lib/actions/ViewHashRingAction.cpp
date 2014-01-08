/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ViewHashRingAction.h"

#include "consistent_hashing/IHashRing.h"

ViewHashRingAction::ViewHashRingAction(const IHashRing& ring, IByteStream& writer)
	: MessageAction(ring.toString(), writer)
{
}
