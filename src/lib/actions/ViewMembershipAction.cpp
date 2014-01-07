/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ViewMembershipAction.h"

#include "membership/IMembership.h"

ViewMembershipAction::ViewMembershipAction(const IMembership& membership, IByteStream& writer)
	: MessageAction(membership.toString(), writer)
{
}
