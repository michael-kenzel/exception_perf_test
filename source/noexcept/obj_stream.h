#ifndef INCLUDED_OBJ_STREAM
#define INCLUDED_OBJ_STREAM

#pragma once

#include <utility>
#include <cstdlib>
#include <charconv>
#include <string_view>
#include <cstdio>

#include "obj.h"


namespace OBJ
{
	class Stream
	{
		const char* ptr;
		const char* end;
		float size;
		int line = 1;
		const char* name;

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

		void endLine() noexcept
		{
			if (line % 0x4000 == 0)
				callback.progress(1.0f - (end - ptr) / size);
			++line;
		}

		void endFile() noexcept
		{
			callback.finish();
		}

	public:
		Stream(const char* begin, const char* end, const char* name, StreamCallback& callback) noexcept
			: ptr(begin), end(end), size(static_cast<float>(end - begin)), name(name), callback(callback)
		{
		}

		void error(const char* msg) const
		{
			callback.error(name, line, msg);
		}

		void warn(const char* msg) const
		{
			callback.warning(name, line, msg);
		}

		bool skipLine() noexcept
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
		bool consume() noexcept
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

		template <char C>
		bool expect() noexcept
		{
			if (!consume<C>())
			{
				constexpr const char msg[] = { 'e', 'x', 'p', 'e', 'c', 't', 'e', 'd', '\'', C, '\'', '\0' };
				error(msg);
			}
		}

		bool consumeHorizontalWS() noexcept
		{
			if (ptr == end || (*ptr != ' ' && *ptr != '\t' && *ptr != '\r'))
				return false;

			do ++ptr; while (ptr != end && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r'));

			return true;
		}

		void expectHorizontalWS() noexcept
		{
			if (!consumeHorizontalWS())
				error("expected horizontal white space");
		}

		bool finishLine() noexcept
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

		void expectLineEnd() noexcept
		{
			if (!finishLine())
				error("expected newline");
		}

		std::string_view consumeNonWS() noexcept
		{
			auto begin = ptr;
			while (ptr != end && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n')
				++ptr;
			return { begin, static_cast<std::size_t>(ptr - begin) };
		}

		std::string_view expectNonWS() noexcept
		{
			auto v = consumeNonWS();
			if (v.empty())
				error("expected string");
			return v;
		}

		bool consumeInteger(int& n) noexcept
		{
			auto [token_end, err] = std::from_chars(ptr, end, n);

			if (static_cast<bool>(err))
			{
				if (err == std::errc::result_out_of_range)
					error("decimal number out of range");
				return false;
			}

			ptr = token_end;

			return true;
		}

		int expectInteger() noexcept
		{
			int n;
			auto [token_end, err] = std::from_chars(ptr, end, n);

			if (static_cast<bool>(err))
			{
				if (err == std::errc::result_out_of_range)
					error("decimal number out of range");
				error("expected decimal number");
			}

			ptr = token_end;

			return n;
		}

		bool consumeFloat(float& f) noexcept
		{
#ifdef _MSC_VER
			auto [token_end, err] = std::from_chars(ptr, end, f);

			if (static_cast<bool>(err))
			{
				if (err == std::errc::result_out_of_range)
					error("floating point number out of range");
				return false;
			}

			ptr = token_end;

			return true;
#else  // WORKAROUND for lack of std::from_chars in gcc and clang
			char* token_end;
			f = std::strtof(ptr, &token_end);

			if (token_end == ptr)
				return false;

			ptr = token_end;

			return true;
#endif
		}

		float expectFloat() noexcept
		{
#ifdef _MSC_VER
			float f;
			auto [token_end, err] = std::from_chars(ptr, end, f);

			if (static_cast<bool>(err))
			{
				if (err == std::errc::result_out_of_range)
					error("floating point number out of range");
				error("expected floating point number");
			}

			ptr = token_end;

			return f;
#else  // WORKAROUND for lack of std::from_chars in gcc and clang
			char* token_end;
			float f = std::strtof(ptr, &token_end);

			if (token_end == ptr)
				error("expected floating point number");

			ptr = token_end;

			return f;
#endif
		}

		template <typename Consumer>
		[[nodiscard]]
		OBJ::error consume(Consumer&& consumer) noexcept
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
			return OBJ::error::SUCCESS
		}
	};
}

#endif  // INCLUDED_OBJ_STREAM
