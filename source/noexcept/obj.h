#ifndef INCLUDED_OBJ
#define INCLUDED_OBJ

#pragma once

#include <array>
#include <vector>

#include <math/vector.h>

#include "dynamic_array.h"


namespace OBJ
{
	enum class error
	{
		SUCCESS = 0,
		FAILED_TO_OPEN_FILE,
		FAILED_TO_READ_FILE,
		SYNTAX_ERROR,
		UNSUPPORTED_FEATURE,
		ALLOCATION_FAILED
	};

	const char* describeError(error) noexcept;


	struct StreamCallback
	{
		virtual void progress(float progress) noexcept = 0;
		virtual void warning(const char* file, int line, const char* msg) noexcept = 0;
		virtual void error(const char* file, int line, const char* msg) noexcept = 0;
		virtual void finish() noexcept = 0;

	protected:
		StreamCallback() = default;
		StreamCallback(StreamCallback&&) = default;
		StreamCallback(const StreamCallback&) = default;
		StreamCallback& operator =(StreamCallback&&) = default;
		StreamCallback& operator =(const StreamCallback&) = default;
		~StreamCallback() = default;
	};


	struct Triangles
	{
		dynamic_array<float3> positions;
		dynamic_array<float3> normals;
		dynamic_array<float2> texcoords;
		dynamic_array<std::array<int, 3>> triangles;
	};

	error readTriangles(Triangles& out, const char* begin, const char* end, const char* name, StreamCallback& stream_callback) noexcept;
	error readTrianglesFromFile(Triangles& out, const char* path, StreamCallback& stream_callback) noexcept;
}

#endif  // INCLUDED_OBJ
