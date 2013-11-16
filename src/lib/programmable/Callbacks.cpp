#include "Callbacks.h"

#include "ForwardToPeer.h"
#include <functional>
using std::bind;
using namespace std::placeholders;

Callbacks::Callbacks(const TurboApi& instruct)
	: TurboApi(instruct)
{
}

void Callbacks::initialize(const IMembership& membership, IPeerTracker& peers)
{
	if (!when_local_write_finishes)
	{
		std::shared_ptr<ForwardToPeer> cmd(new ForwardToPeer(membership, peers));
		when_local_write_finishes = bind(&ForwardToPeer::run, cmd, _1, _2);
	}
}
