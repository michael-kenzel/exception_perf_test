#ifndef INCLUDED_OBJ_READER
#define INCLUDED_OBJ_READER

#pragma once

#include "obj_stream.h"


namespace OBJ
{
	template <typename Consumer>
	class Reader
	{
		Consumer& consumer;

		[[nodiscard]]
		bool consumeVertex(OBJ::Stream& stream) noexcept
		{
			if (auto x = stream.expectFloat(); x && stream.expectHorizontalWS())
			{
				if (auto y = stream.expectFloat(); y && stream.expectHorizontalWS())
				{
					if (auto z = stream.expectFloat())
					{
						if (float w; stream.consumeHorizontalWS() && stream.consumeFloat(w))
						{
							if (!stream.expectLineEnd())
								return false;
							return consumer.consumeVertex(stream, *x, *y, *z, w);
						}

						if (!stream.expectLineEnd())
							return false;
						return consumer.consumeVertex(stream, *x, *y, *z);
					}
				}
			}

			return false;
		}

		[[nodiscard]]
		bool consumeNormal(OBJ::Stream& stream) noexcept
		{
			if (auto x = stream.expectFloat(); x && stream.expectHorizontalWS())
			{
				if (auto y = stream.expectFloat(); y && stream.expectHorizontalWS())
				{
					if (auto z = stream.expectFloat(); z && stream.expectLineEnd())
					{
						return consumer.consumeNormal(stream, *x, *y, *z);
					}
				}
			}

			return false;
		}

		[[nodiscard]]
		bool consumeTexcoord(OBJ::Stream& stream) noexcept
		{
			if (auto u = stream.expectFloat())
			{
				if (float v; stream.consumeHorizontalWS() && stream.consumeFloat(v))
				{
					if (float w; stream.consumeHorizontalWS() && stream.consumeFloat(w))
					{
						if (!stream.expectLineEnd())
							return false;
						return consumer.consumeTexcoord(stream, *u, v, w);
					}

					if (!stream.expectLineEnd())
						return false;
					return consumer.consumeTexcoord(stream, *u, v);
				}

				if (!stream.expectLineEnd())
					return false;
				return consumer.consumeTexcoord(stream, *u);
			}

			return false;
		}

		[[nodiscard]]
		bool consumeFace(OBJ::Stream& stream) noexcept
		{
			do
			{
				auto v = stream.expectInteger();

				if (!v)
					return false;

				int t = 0;
				int n = 0;

				if (stream.consume<'/'>())
				{
					stream.consumeInteger(t);

					if (stream.consume<'/'>())
						stream.consumeInteger(n);
				}

				if (!consumer.consumeFaceVertex(stream, v, n, t))
					return false;
			} while (!stream.finishLine());

			return consumer.finishFace(stream);
		}

		[[nodiscard]]
		bool consumeObjectName(OBJ::Stream& stream) noexcept
		{
			if (auto name = stream.expectNonWS())
			{
				while (!stream.finishLine())
				{
					auto n = stream.consumeNonWS();

					if (!n)
						return false;

					*name = { &*name[0], static_cast<std::size_t>((&*n[0] + size(n)) - &*name[0]) };
				}

				return consumer.consumeObjectName(stream, *name);
			}

			return false;
		}

		[[nodiscard]]
		bool consumeGroupName(OBJ::Stream& stream) noexcept
		{
			do
			{
				if (auto name = stream.expectNonWS())
				{
					if (!consumer.consumeGroupName(stream, *name))
						return false;
				}
			} while (!stream.finishLine());

			return consumer.finishGroupAssignment(stream);
		}

		[[nodiscard]]
		bool consumeSmoothingGroup(OBJ::Stream& stream) noexcept
		{
			int n;

			if (!stream.consumeInteger(n))
			{
				if (stream.consume<'o', 'f', 'f'>())
				{
					n = 0;
				}
				else
				{
					stream.error("expected smoothing group index or 'off'");
					return false;
				}
			}

			if (!stream.expectLineEnd())
				return false;

			return consumer.consumeSmoothingGroup(stream, n);
		}

		[[nodiscard]]
		bool consumeMtlLib(OBJ::Stream& stream) noexcept
		{
			if (auto name = stream.expectNonWS(); name && stream.expectLineEnd())
				return consumer.consumeMtlLib(stream, *name);
			return false;
		}

		[[nodiscard]]
		bool consumeUseMtl(OBJ::Stream& stream) noexcept
		{
			if (auto name = stream.expectNonWS(); name && stream.expectLineEnd())
				return consumer.consumeUseMtl(stream, name);
			return false;
		}

	public:
		Reader(Consumer& consumer) noexcept
			: consumer(consumer)
		{
		}

		[[nodiscard]]
		bool consume(OBJ::Stream& stream, char c) noexcept
		{
			switch (c)
			{
			case 'v':
				if (stream.consumeHorizontalWS())
				{
					if (!consumeVertex(stream))
						return false;
					break;
				}
				else if (stream.consume<'n'>())
				{
					if (stream.consumeHorizontalWS())
					{
						if (!consumeNormal(stream))
							return false;
						break;
					}
				}
				else if (stream.consume<'t'>())
				{
					if (stream.consumeHorizontalWS())
					{
						if (!consumeTexcoord(stream))
							return false;
						break;
					}
				}
				[[fallthrough]];

			case 'f':
				if (stream.consumeHorizontalWS())
				{
					if (!consumeFace(stream))
						return false;
					break;
				}
				[[fallthrough]];

			case 'o':
				if (stream.consumeHorizontalWS())
				{
					if (!consumeObjectName(stream))
						return false;
					break;
				}
				[[fallthrough]];

			case 'g':
				if (stream.consumeHorizontalWS())
				{
					if (!consumeGroupName(stream))
						return false;
					break;
				}
				[[fallthrough]];

			case 's':
				if (stream.consumeHorizontalWS())
				{
					if (!consumeSmoothingGroup(stream))
						return false;
					break;
				}
				[[fallthrough]];

			case 'm':
				if (stream.consume<'t', 'l', 'l', 'i', 'b'>())
				{
					if (stream.consumeHorizontalWS())
						if (!consumeMtlLib(stream))
							return false;
					break;
				}
				[[fallthrough]];

			case 'u':
				if (stream.consume<'s', 'e', 'm', 't', 'l'>())
				{
					if (stream.consumeHorizontalWS())
						if (!consumeUseMtl(stream))
							return false;
					break;
				}
				[[fallthrough]];

			default:
				stream.error("unknown command");
				return false;

			case '#':
				stream.skipLine();
				break;
			}

			return true;
		}
	};
}

#endif  // INCLUDED_OBJ_READER
