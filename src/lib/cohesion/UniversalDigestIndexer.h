/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IDigestIndexer.h"
#include "DigestTree.h"

class UniversalDigestIndexer : public IDigestIndexer
{
public:
	UniversalDigestIndexer();

	void add(const std::string& key);
	void remove(const std::string& key);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IDigestKeys& find(const std::string& id) const;
	const IDigestKeys& randomTree() const;
	const IDigestKeys& unwantedTree() const;

protected:
	DigestTree _tree;
};
