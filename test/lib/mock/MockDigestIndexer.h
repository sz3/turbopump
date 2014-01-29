/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "MockDigestTree.h"
#include "cohesion/IDigestIndexer.h"
#include "util/CallHistory.h"

class MockDigestIndexer : public IDigestIndexer
{
public:
	void add(const std::string& key);
	void remove(const std::string& key);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IDigestKeys& find(const std::string& id) const;
	const IDigestKeys& randomTree() const;
	const IDigestKeys& unwantedTree() const;

public:
	MockDigestTree _tree;
	mutable CallHistory _history;
};
