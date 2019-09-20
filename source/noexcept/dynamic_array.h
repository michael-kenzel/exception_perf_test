#ifndef INCLUDED_DYNAMIC_ARRAY
#define INCLUDED_DYNAMIC_ARRAY

#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>
#include <initializer_list>
#include <new>
#include <memory>
#include <algorithm>


template <typename T>
class dynamic_array
{
public:

private:
	struct element_storage_t
	{
		union
		{
			T t;
		};

		template <typename... Args>
		T* construct(Args&&... args) noexcept
		{
			return new (this) T(std::forward<Args>(args)...);
		}

		void destruct() noexcept
		{
			t.~T();
		}
	};

	std::unique_ptr<element_storage_t[]> buffer;
	std::size_t size = 0;
	std::size_t capacity = 0;

	void moveContent(std::unique_ptr<element_storage_t[]>&& new_buffer) noexcept
	{
		if constexpr (std::is_nothrow_move_constructible_v<T>)
		{
			for (std::size_t i = 0; i < size; ++i)
				new_buffer[i].construct(std::move(buffer[i].t));
		}
		else
		{
			static_assert(std::is_nothrow_copy_constructible_v<T>);

			for (std::size_t i = 0; i < size; ++i)
				new_buffer[i].construct(buffer[i].t);

			destroyContent();
		}

		buffer = std::move(new_buffer);
	}

	void destroyContent() noexcept
	{
		for (auto p = &buffer[0] + size; p >= &buffer[0]; --p)
			p->destruct();
	}


	bool grow(std::size_t new_size) noexcept
	{
		if (new_size > capacity)
		{
			auto new_buffer = std::unique_ptr<element_storage_t[]> { new (std::nothrow) element_storage_t[new_size] };
			if (!new_buffer)
				return false;
			moveContent(std::move(new_buffer));
		}

		return true;
	}

public:
	~dynamic_array()
	{
		destroyContent();
	}

	bool push_back(const T&) noexcept
	{
		if (!grow(size + 1))
			return false;
		return true;
	}

	template <typename... Args>
	bool emplace_back(Args&&... args) noexcept
	{
		if (!grow(size + 1))
			return false;
		return true;
	}

	const T& operator [](std::size_t i) const noexcept
	{
		return buffer[i].t;
	}

	T& operator [](std::size_t i) noexcept
	{
		return buffer[i].t;
	}

	friend std::size_t size(const dynamic_array& arr) noexcept
	{
		return arr.size;
	}
};

#endif  // INCLUDED_DYNAMIC_ARRAY
