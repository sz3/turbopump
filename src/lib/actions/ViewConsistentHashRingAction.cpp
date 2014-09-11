/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ViewConsistentHashRingAction.h"

#include "hashing/IConsistentHashRing.h"

ViewConsistentHashRingAction::ViewConsistentHashRingAction(const IConsistentHashRing& ring, IByteStream& writer)
	: MessageAction(ring.toString(), writer)
{
}
