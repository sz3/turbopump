/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "MessageAction.h"

class IByteStream;
class IMembership;

class ViewMembershipAction : public MessageAction
{
public:
	ViewMembershipAction(const IMembership& membership, IByteStream& writer);
};

