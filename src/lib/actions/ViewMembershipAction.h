#pragma once

#include "MessageAction.h"

class IByteStream;
class IMembership;

class ViewMembershipAction : public MessageAction
{
public:
	ViewMembershipAction(const IMembership& membership, IByteStream& writer);
};

