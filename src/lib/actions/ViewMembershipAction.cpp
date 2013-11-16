#include "ViewMembershipAction.h"

#include "membership/IMembership.h"

ViewMembershipAction::ViewMembershipAction(const IMembership& membership, IByteStream& writer)
	: MessageAction(membership.toString(), writer)
{
}
