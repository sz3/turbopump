/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IKeyTabulator.h"
#include "IDigestIndexer.h"
#include <map>
#include <memory>
class ILocateKeys;

// a forest of merkle trees.
// organized by section, and tracking ones we want to talk about with our peers "wanted"
// and those we want to offload ASAP ("unwanted")

class KeyTabulator : public IKeyTabulator
{
public:
	KeyTabulator(const ILocateKeys& locator);

	void update(const std::string& key, unsigned long long value, unsigned mirrors=3);
	void remove(const std::string& key, unsigned mirrors=3);

	void splitSection(const std::string& where);
	void cannibalizeSection(const std::string& where);

	const IDigestKeys& find(const std::string& id, unsigned mirrors=3) const;
	const IDigestKeys& randomTree() const;
	const IDigestKeys& unwantedTree() const;

	void print() const; // for testing

protected:
	const ILocateKeys& _locator;
	std::map<unsigned char, std::unique_ptr<IDigestIndexer>> _forest;
};
