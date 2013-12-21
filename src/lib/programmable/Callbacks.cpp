#include "Callbacks.h"

#include "ForwardToPeer.h"
#include "cohesion/IMerkleIndex.h"
#include <deque>
#include <functional>
using std::bind;
using std::deque;
using std::function;
using namespace std::placeholders;

// TODO: rather than anonymous namespace, should split these functions out somewhere else...
namespace
{
	auto merkleAddFunct(IMerkleIndex& merkleIndex)
	{
		return [&] (std::string filename, IDataStoreReader::ptr contents)
		{
			merkleIndex.add(filename);
		};
	}

	auto writeChainFunct(const IMembership& membership, IPeerTracker& peers)
	{
		std::shared_ptr<ForwardToPeer> cmd(new ForwardToPeer(membership, peers));
		return bind(&ForwardToPeer::run, cmd, _1, _2);
	}
}

Callbacks::Callbacks()
{
}

Callbacks::Callbacks(const TurboApi& instruct)
	: TurboApi(instruct)
{
}

void Callbacks::initialize(const IMembership& membership, IPeerTracker& peers, IMerkleIndex& merkleIndex)
{
	// TODO: devise a proper callback strategy for configurable default callbacks + user defined ones.
	//  yes, I know this is basically: "TODO: figure out how to land on moon"

	// on local write
	{
		deque<function<void(std::string, IDataStoreReader::ptr)>> functionChainer;
		if (TurboApi::options.merkle)
			functionChainer.push_back( merkleAddFunct(merkleIndex) );

		if (TurboApi::options.write_chaining)
			functionChainer.push_back( writeChainFunct(membership, peers) );

		if (when_local_write_finishes)
			functionChainer.push_back(when_local_write_finishes);

		if (!functionChainer.empty())
		{
			when_local_write_finishes = [functionChainer] (std::string filename, IDataStoreReader::ptr contents)
			{
				for (auto fun : functionChainer)
					fun(filename, contents);
			};
		}
	}

	// on mirror write
	{
		deque<function<void(std::string, IDataStoreReader::ptr)>> functionChainer;
		if (TurboApi::options.merkle)
			functionChainer.push_back( merkleAddFunct(merkleIndex) );

		if (when_mirror_write_finishes)
			functionChainer.push_back(when_mirror_write_finishes);

		if (!functionChainer.empty())
		{
			when_mirror_write_finishes = [functionChainer] (std::string filename, IDataStoreReader::ptr contents)
			{
				for (auto fun : functionChainer)
					fun(filename, contents);
			};
		}
	}
}
