#ifndef INCLUDED_OBJ_STREAM
#define INCLUDED_OBJ_STREAM

#pragma once

#include <utility>
#include <cstdlib>
#include <charconv>
#include <string_view>
#include <iterator>
#include <iostream>

#include "obj.h"

using namespace std::literals;


namespace OBJ
{
	class Stream
	{
		const char* ptr;
		const char* end;
		float size;
		int line = 1;
		std::string_view name;

		StreamCallback& callback;


		static constexpr bool isHorizontalWS(char c)
		{
			return c == ' ' || c == '\t' || c == '\r';
		}

		static constexpr bool isVerticalWS(char c)
		{
			return c == '\n' || c == '\v' || c == '\f';
		}

		static constexpr bool isWS(char c)
		{
			return isHorizontalWS(c) || isVerticalWS(c);
		}

		void endLine()
		{
			if (line % 0x4000 == 0)
				callback.progress(1.0f - (end - ptr) / size);
			++line;
		}

		void endFile()
		{
			callback.finish();
		}

	public:
		Stream(const char* begin, const char* end, std::string_view name, StreamCallback& callback)
			: ptr(begin), end(end), size(static_cast<float>(end - begin)), name(name), callback(callback)
		{
		}

		[[noreturn]]
		void throwError(std::string_view msg) const
		{
			callback.error(name, line, msg);
			throw OBJ::parse_error();
		}

		void warn(std::string_view msg) const
		{
			callback.warning(name, line, msg);
		}

		bool skipLine()
		{
			while (ptr != end)
			{
				if (*ptr++ == '\n')
				{
					endLine();
					return true;
				}
			}
			return false;
		}

		template <char... C>
		bool consume()
		{
			static_assert(sizeof...(C) > 0);
			static_assert(((!isWS(C)) && ...), "consume does not support whitespace characters");

			if (auto c = ptr; ptr + sizeof...(C) < end && ((*c++ == C) && ...))
			{
				ptr = c;
				return true;
			}
			return false;
		}

		//template <char C>
		//void expect()
		//{
		//	if (!consume<C>())
		//	{
		//		constexpr const char msg[] = { 'e', 'x', 'p', 'e', 'c', 't', 'e', 'd', '\'', C, '\'' };
		//		throwError({ msg, std::size(msg) });
		//	}
		//}

		bool consumeHorizontalWS()
		{
			if (ptr == end || !isHorizontalWS(*ptr))
				return false;

			while (++ptr, ptr != end && isHorizontalWS(*ptr));

			return true;
		}

		void expectHorizontalWS()
		{
			if (!consumeHorizontalWS())
				throwError("expected horizontal white space"sv);
		}

		bool finishLine()
		{
			consumeHorizontalWS();

			if (ptr == end)
				return true;

			if (*ptr == '\n')
			{
				++ptr;
				endLine();
				return true;
			}

			return false;
		}

		void expectLineEnd()
		{
			if (!finishLine())
				throwError("expected newline"sv);
		}

		std::string_view consumeNonWS()
		{
			auto begin = ptr;
			while (ptr != end && !isWS(*ptr))
				++ptr;
			return { begin, static_cast<std::size_t>(ptr - begin) };
		}

		std::string_view expectNonWS()
		{
			auto v = consumeNonWS();
			if (v.empty())
				throwError("expected string"sv);
			return v;
		}

		bool consumeInteger(int& n)
		{
			auto [token_end, err] = std::from_chars(ptr, end, n);

			if (err != std::errc())
			{
				if (err == std::errc::result_out_of_range)
					throwError("integer out of range"sv);
				return false;
			}

			ptr = token_end;

			return true;
		}

		int expectInteger()
		{
			if (int n; consumeInteger(n))
				return n;
			throwError("expected integer"sv);
		}

		bool consumeFloat(float& f)
		{
			auto [token_end, err] = std::from_chars(ptr, end, f);

			if (err != std::errc())
			{
				if (err == std::errc::result_out_of_range)
					throwError("floating point number out of range"sv);
				return false;
			}

			ptr = token_end;

			return true;
		}

		float expectFloat()
		{
			if (float f; consumeFloat(f))
				return f;
			throwError("expected floating point number"sv);
		}

		template <typename Consumer>
		void consume(Consumer&& consumer)
		{
			while (ptr != end)
			{
				char c = *ptr++;

				switch (c)
				{
				case '\n':
					endLine();
				case '\r':
				case '\t':
				case ' ':
					break;

				default:
					if (!consumer.consume(*this, c))
						return;
					break;
				}
			}

			endFile();
		}
	};
}

#endif  // INCLUDED_OBJ_STREAM
