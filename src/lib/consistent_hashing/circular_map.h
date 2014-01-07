/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <cstddef>
#include <map>
#include <utility>

// TODO: probably needs to be an unordered_map or concurrent_map, right?
template <typename KT, typename VT>
class circular_map
{
public:
	class iterator
	{
	public:
		iterator(const circular_map& ring, const typename std::map<KT,VT>::const_iterator& origin)
			: _current(origin)
			, _begin(ring._map.begin())
			, _end(ring._map.end())
		{
			checkLoop();
			_origin = _current;
		}

		// alternate constructor for comparisons, such as map.end()
		iterator(const typename std::map<KT,VT>::const_iterator& node)
			: _current(node)
			, _origin(node)
			, _begin(node)
			, _end(node)
		{
		}

		const typename std::pair<const KT,VT>* operator->() const
		{
		   return _current.operator->();
		}

		iterator& operator++()
		{
			++_current;
			checkLoop();
			checkEnd();
			return *this;
		}

		bool operator!=(const iterator& rhs) const
		{
			return _current != rhs._current;
		}

		bool operator==(const iterator& rhs) const
		{
			return !operator!=(rhs);
		}

		bool bad() const
		{
			return _current == _end;
		}

	protected:
		void checkLoop()
		{
			if (_current == _end)
				_current = _begin;
		}

		void checkEnd()
		{
			if (_current == _origin)
				_current = _end;
		}

	protected:
		typename std::map<KT,VT>::const_iterator _origin;
		typename std::map<KT,VT>::const_iterator _current;
		typename std::map<KT,VT>::const_iterator _begin;
		typename std::map<KT,VT>::const_iterator _end;
	};

public:
	circular_map()
		: _end(iterator(_map.end()))
	{
	}

	VT& operator[](const KT& key)
	{
		return _map[key];
	}

	void insert(const KT& key, const VT& value)
	{
		_map.insert( std::pair<KT,VT>(key, value) );
	}

	size_t erase(const KT& key)
	{
		return _map.erase(key);
	}

	iterator lower_bound(const KT& key) const
	{
		return iterator(*this, _map.lower_bound(key));
	}

	bool empty() const
	{
		return _map.empty();
	}

	const iterator& end() const
	{
		return _end;
	}

	size_t size() const
	{
		return _map.size();
	}

protected:
	std::map<KT, VT> _map;
	const iterator   _end;
};
