#ifndef INCLUDED_HASH_MAP
#define INCLUDED_HASH_MAP

#pragma once

#include <cstddef>
#include <utility>
#include <new>
#include <memory>
#include <optional>
#include <functional>
#include <algorithm>

#include "dynamic_array.h"


template <typename Key, typename Value, typename Hash = std::hash<Key>>
class hash_map
{
public:
	using key_type = Key;
	using mapped_type = Value;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using hasher = Hash;
	using value_type = std::pair<const Key, Value>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;

private:
	dynamic_array<value_type> table;

	bool resize()
	{
		return table.size() * 4 > table.capacity() * 3;
	}

public:
	template <typename... Args>
	std::optional<std::pair<pointer, bool>> try_emplace(const key_type& key, Args&&... args)
	{
		return {};
	}

};

#endif  // INCLUDED_HASH_MAP
