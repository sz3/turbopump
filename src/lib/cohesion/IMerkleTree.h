/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/MerklePoint.h"
#include <deque>
#include <string>

// need to account for the future with multiple merkle trees contained herein
// for the sake of everyone's sanity (e.g. mine), we'll need a standard approach for what
// part of the cyptographic hash to use to organize the merkle tree.
// notably, with DHT buckets using the high-end bits to distribute keys, it may be important to use
// low end bits for synchonization and in memory/on disk key organization. That way we can use the high
// bits for cross-worker organization, and the low bits for better on-worker distribution.
// (and a semi-random distribution will be very important for the merkle tree)

// the DataStore will use a separate hashing algorithm, because (I'm hoping) iterating it in order
// will not be necessary or beneficial. Operations will be on a per-key basis regardless, so
// order is hopefully irrelevant. That will be the assumption going forward, to allow us to
// prioritize performance over other concerns in that area of the system.

class IMerkleTree
{
public:
	virtual ~IMerkleTree() {}
	virtual std::string id() const = 0;

	virtual void add(const std::string& key) = 0;
	virtual void remove(const std::string& key) = 0;

	virtual bool empty() const = 0;
	virtual MerklePoint top() const = 0;
	virtual std::deque<MerklePoint> diff(const MerklePoint& point) const = 0;
	virtual std::deque<std::string> enumerate(unsigned long long first, unsigned long long last, unsigned limit=50) const = 0;
};
