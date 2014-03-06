/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "UniversalDigestIndexer.h"

using std::string;

UniversalDigestIndexer::UniversalDigestIndexer()
{
	TreeId id("", 0);
	_tree.setId(id);
}

void UniversalDigestIndexer::update(const std::string& key, unsigned long long value)
{
	_tree.update(key, value);
}

void UniversalDigestIndexer::remove(const string& key)
{
	_tree.remove(key);
}

void UniversalDigestIndexer::splitSection(const string& where)
{
}

void UniversalDigestIndexer::cannibalizeSection(const string& where)
{
}

const IDigestKeys& UniversalDigestIndexer::find(const string& id) const
{
	return _tree;
}

const IDigestKeys& UniversalDigestIndexer::randomTree() const
{
	return _tree;
}

const IDigestKeys& UniversalDigestIndexer::unwantedTree() const
{
	return DigestTree::null();
}
