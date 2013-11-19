#include "Callbacks.h"

#include "ForwardToPeer.h"
#include "cohesion/IMerkleIndex.h"
#include <functional>
using std::bind;
using namespace std::placeholders;

Callbacks::Callbacks(const TurboApi& instruct)
	: TurboApi(instruct)
{
}

void Callbacks::initialize(const IMembership& membership, IPeerTracker& peers, IMerkleIndex& merkleIndex)
{
	if (!when_local_write_finishes)
	{
		std::shared_ptr<ForwardToPeer> cmd(new ForwardToPeer(membership, peers));
		when_local_write_finishes = bind(&ForwardToPeer::run, cmd, _1, _2);
	}

	// TODO: devise a proper callback strategy for configurable default callbacks + user defined ones.
	//  yes, I know this is basically: "TODO: figure out how to land on moon"
	{
		auto userFun = when_local_write_finishes;
		when_local_write_finishes = [&merkleIndex,userFun] (std::string filename, IDataStoreReader::ptr contents)
		{
			merkleIndex.add(filename);
			if (userFun)
				userFun(std::move(filename), std::move(contents));
		};
	}
}
