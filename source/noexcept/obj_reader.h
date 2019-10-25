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
		OBJ::error consumeVertex(OBJ::Stream& stream) noexcept
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
								return OBJ::error::SYNTAX_ERROR;
							return consumer.consumeVertex(stream, *x, *y, *z, w);
						}

						if (!stream.expectLineEnd())
							return OBJ::error::SYNTAX_ERROR;
						return consumer.consumeVertex(stream, *x, *y, *z);
					}
				}
			}

			return OBJ::error::SYNTAX_ERROR;
		}

		[[nodiscard]]
		OBJ::error consumeNormal(OBJ::Stream& stream) noexcept
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

			return OBJ::error::SYNTAX_ERROR;
		}

		[[nodiscard]]
		OBJ::error consumeTexcoord(OBJ::Stream& stream) noexcept
		{
			if (auto u = stream.expectFloat())
			{
				if (float v; stream.consumeHorizontalWS() && stream.consumeFloat(v))
				{
					if (float w; stream.consumeHorizontalWS() && stream.consumeFloat(w))
					{
						if (!stream.expectLineEnd())
							return OBJ::error::SYNTAX_ERROR;
						return consumer.consumeTexcoord(stream, *u, v, w);
					}

					if (!stream.expectLineEnd())
						return OBJ::error::SYNTAX_ERROR;
					return consumer.consumeTexcoord(stream, *u, v);
				}

				if (!stream.expectLineEnd())
					return OBJ::error::SYNTAX_ERROR;
				return consumer.consumeTexcoord(stream, *u);
			}

			return OBJ::error::SYNTAX_ERROR;
		}

		[[nodiscard]]
		OBJ::error consumeFace(OBJ::Stream& stream) noexcept
		{
			do
			{
				auto v = stream.expectInteger();

				if (!v)
					return OBJ::error::SYNTAX_ERROR;

				int t = 0;
				int n = 0;

				if (stream.consume<'/'>())
				{
					stream.consumeInteger(t);

					if (stream.consume<'/'>())
						stream.consumeInteger(n);
				}

				if (auto ret = consumer.consumeFaceVertex(stream, *v, n, t); ret != OBJ::error::SUCCESS)
					return ret;
			} while (!stream.finishLine());

			return consumer.finishFace(stream);
		}

		[[nodiscard]]
		OBJ::error consumeObjectName(OBJ::Stream& stream) noexcept
		{
			if (auto name = stream.expectNonWS())
			{
				while (!stream.finishLine())
				{
					auto n = stream.consumeNonWS();
					*name = { &(*name)[0], static_cast<std::size_t>((&n[0] + size(n)) - &(*name)[0]) };
				}

				return consumer.consumeObjectName(stream, *name);
			}

			return OBJ::error::SYNTAX_ERROR;
		}

		[[nodiscard]]
		OBJ::error consumeGroupName(OBJ::Stream& stream) noexcept
		{
			do
			{
				auto name = stream.expectNonWS();

				if (!name)
					return OBJ::error::SYNTAX_ERROR;

				if (auto ret = consumer.consumeGroupName(stream, *name); ret != OBJ::error::SUCCESS)
					return ret;
			} while (!stream.finishLine());

			return consumer.finishGroupAssignment(stream);
		}

		[[nodiscard]]
		OBJ::error consumeSmoothingGroup(OBJ::Stream& stream) noexcept
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
					return OBJ::error::SYNTAX_ERROR;
				}
			}

			if (!stream.expectLineEnd())
				return OBJ::error::SYNTAX_ERROR;

			return consumer.consumeSmoothingGroup(stream, n);
		}

		[[nodiscard]]
		OBJ::error consumeMtlLib(OBJ::Stream& stream) noexcept
		{
			if (auto name = stream.expectNonWS(); name && stream.expectLineEnd())
				return consumer.consumeMtlLib(stream, *name);
			return OBJ::error::SYNTAX_ERROR;
		}

		[[nodiscard]]
		OBJ::error consumeUseMtl(OBJ::Stream& stream) noexcept
		{
			if (auto name = stream.expectNonWS(); name && stream.expectLineEnd())
				return consumer.consumeUseMtl(stream, *name);
			return OBJ::error::SYNTAX_ERROR;
		}

	public:
		Reader(Consumer& consumer) noexcept
			: consumer(consumer)
		{
		}

		[[nodiscard]]
		OBJ::error consume(OBJ::Stream& stream, char c) noexcept
		{
			switch (c)
			{
			case 'v':
				if (stream.consumeHorizontalWS())
				{
					if (auto ret = consumeVertex(stream); ret != OBJ::error::SUCCESS)
						return ret;
					break;
				}
				else if (stream.consume<'n'>())
				{
					if (stream.consumeHorizontalWS())
					{
						if (auto ret = consumeNormal(stream); ret != OBJ::error::SUCCESS)
							return ret;
						break;
					}
				}
				else if (stream.consume<'t'>())
				{
					if (stream.consumeHorizontalWS())
					{
						if (auto ret = consumeTexcoord(stream); ret != OBJ::error::SUCCESS)
							return ret;
						break;
					}
				}
				[[fallthrough]];

			case 'f':
				if (stream.consumeHorizontalWS())
				{
					if (auto ret = consumeFace(stream); ret != OBJ::error::SUCCESS)
						return ret;
					break;
				}
				[[fallthrough]];

			case 'o':
				if (stream.consumeHorizontalWS())
				{
					if (auto ret = consumeObjectName(stream); ret != OBJ::error::SUCCESS)
						return ret;
					break;
				}
				[[fallthrough]];

			case 'g':
				if (stream.consumeHorizontalWS())
				{
					if (auto ret = consumeGroupName(stream); ret != OBJ::error::SUCCESS)
						return ret;
					break;
				}
				[[fallthrough]];

			case 's':
				if (stream.consumeHorizontalWS())
				{
					if (auto ret = consumeSmoothingGroup(stream); ret != OBJ::error::SUCCESS)
						return ret;
					break;
				}
				[[fallthrough]];

			case 'm':
				if (stream.consume<'t', 'l', 'l', 'i', 'b'>())
				{
					if (stream.consumeHorizontalWS())
						if (auto ret = consumeMtlLib(stream); ret != OBJ::error::SUCCESS)
							return ret;
					break;
				}
				[[fallthrough]];

			case 'u':
				if (stream.consume<'s', 'e', 'm', 't', 'l'>())
				{
					if (stream.consumeHorizontalWS())
						if (auto ret = consumeUseMtl(stream); ret != OBJ::error::SUCCESS)
							return ret;
					break;
				}
				[[fallthrough]];

			default:
				stream.error("unknown command");
				return OBJ::error::SYNTAX_ERROR;

			case '#':
				stream.skipLine();
				break;
			}

			return OBJ::error::SUCCESS;
		}
	};
}

#endif  // INCLUDED_OBJ_READER
