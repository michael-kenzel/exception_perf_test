#ifndef INCLUDED_OBJ
#define INCLUDED_OBJ

#pragma once

#include <exception>
#include <array>
#include <vector>
#include <string_view>
#include <filesystem>

#include <math/vector.h>


namespace OBJ
{
	class parse_error : std::exception
	{
	public:
		const char* what() const noexcept
		{
			return "parse error";
		}
	};


	struct StreamCallback
	{
		virtual void progress(float progress) = 0;
		virtual void warning(std::string_view file, int line, std::string_view msg) = 0;
		virtual void error(std::string_view file, int line, std::string_view msg) = 0;
		virtual void finish() = 0;

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
		std::vector<float3> positions;
		std::vector<float3> normals;
		std::vector<float2> texcoords;
		std::vector<std::array<int, 3>> triangles;
	};

	Triangles readTriangles(const char* begin, const char* end, std::string_view name, StreamCallback& stream_callback);
	Triangles readTriangles(const std::filesystem::path& path, StreamCallback& stream_callback);
}

#endif  // INCLUDED_OBJ
