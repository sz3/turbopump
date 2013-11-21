#include "SkewCorrector.h"
#include <deque>
#include <iostream>

void SkewCorrector::healKey(const Peer& peer, unsigned long long key)
{
	std::cout << "how does SkewCorrector heal key? :(" << std::endl;
}

void SkewCorrector::pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	std::cout << "how does SkewCorrector pushKeyRange? :(" << std::endl;

	// need to find all files in the key ranges, and write them to peer.

}
