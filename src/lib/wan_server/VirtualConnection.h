#pragma once

#include "OrderedPacket.h"
#include "actions/IAction.h"
#include <queue>
#include <memory>
#include <string>

// TODO: name this better. It's more an action helper. Buffer. Thing.
class VirtualConnection
{
public:
	void setAction(const std::shared_ptr<IAction>& action);
	const std::shared_ptr<IAction>& action() const;

	void push(OrderedPacket packet);
	bool pop(OrderedPacket& packet);

protected:
	std::shared_ptr<IAction> _action;
	std::priority_queue<OrderedPacket> _pending;
};
