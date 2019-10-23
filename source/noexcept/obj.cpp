#include <utility>
#include <cstdint>
#include <cstring>
#include <memory>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cstdlib>
#include <cstdio>

#include "dynamic_array.h"
#include "hash_map.h"

#include "obj_stream.h"
#include "obj_reader.h"
#include "obj.h"


namespace
{
	std::size_t combineHashes(std::size_t a, std::size_t b) noexcept
	{
		// based on https://stackoverflow.com/a/27952689/2064761
		return a ^ (b + 0x9E3779B9U + (a << 6) + (a >> 2));
	}

	struct face_vertex_t
	{
		int v, n, t;

		friend constexpr bool operator ==(const face_vertex_t& a, const face_vertex_t& b) noexcept
		{
			return a.v == b.v && a.n == b.n && a.t == b.t;
		}
	};

	struct face_vertex_hash : private std::hash<int>
	{
		using std::hash<int>::operator();

		std::size_t operator ()(const face_vertex_t& v) const noexcept
		{
			return combineHashes(combineHashes((*this)(v.v), (*this)(v.n)), (*this)(v.t));
		}
	};


	class OBJConsumer
	{
		dynamic_array<float3> v;
		dynamic_array<float3> vn;
		dynamic_array<float2> vt;

		hash_map<face_vertex_t, int, face_vertex_hash> vertex_map;

		dynamic_array<float3> positions;
		dynamic_array<float3> normals;
		dynamic_array<float2> texcoords;
		dynamic_array<std::array<int, 3>> triangles;

		static constexpr int MAX_FACE_VERTICES = 7;

		int face_vertices[MAX_FACE_VERTICES];
		int num_face_vertices = 0;

	public:
		OBJConsumer() noexcept
			//: vn {{ 0.0f, 0.0f, 0.0f }}, vt {{ 0.0f, 0.0f }}
		{
		}

		[[nodiscard]]
		bool consumeVertex(OBJ::Stream& stream, float x, float y, float z) noexcept
		{
			return v.emplace_back(x, y, z);
		}

		[[nodiscard]]
		bool consumeVertex(OBJ::Stream& stream, float x, float y, float z, float w) noexcept
		{
			stream.error("weighted vertex coordinates are not supported");
			return false;
		}

		[[nodiscard]]
		bool consumeNormal(OBJ::Stream& stream, float x, float y, float z) noexcept
		{
			return vn.emplace_back(x, y, z);
		}

		[[nodiscard]]
		bool consumeTexcoord(OBJ::Stream& stream, float u) noexcept
		{
			stream.error("1D texture coordinates are not supported");
			return false;
		}

		[[nodiscard]]
		bool consumeTexcoord(OBJ::Stream& stream, float u, float v) noexcept
		{
			return vt.emplace_back(u, 1.0f - v);
		}

		[[nodiscard]]
		bool consumeTexcoord(OBJ::Stream& stream, float u, float v, float w) noexcept
		{
			stream.error("3D texture coordinates are not supported");
			return false;
		}

		[[nodiscard]]
		bool consumeFaceVertex(OBJ::Stream& stream, int vi, int ni, int ti) noexcept
		{
			if (vi < 0)
				vi = static_cast<int>(size(v)) + vi;
			else
				--vi;

			if (ni < 0)
				ni = static_cast<int>(size(vn)) + ni;

			if (ti < 0)
				ti = static_cast<int>(size(vt)) + ti;

			auto vertex = vertex_map.try_emplace({ vi, ni, ti }, static_cast<int>(size(positions)));

			if (!vertex)
				return false;

			auto [fv, inserted] = *vertex;

			if (inserted)
			{
				if (!positions.push_back(v[vi]))
					return false;
				if (!normals.push_back(vn[ni]))
					return false;
				if (!texcoords.push_back(vt[ti]))
					return false;
			}

			if (num_face_vertices >= MAX_FACE_VERTICES)
				stream.error("this face has too many vertices");
			face_vertices[num_face_vertices++] = fv->second;
			return true;
		}

		[[nodiscard]]
		bool finishFace(OBJ::Stream& stream) noexcept
		{
			if (num_face_vertices < 3)
				stream.error("face must have at least three vertices");

			for (int i = 2; i < num_face_vertices; ++i)
				if (!triangles.push_back({ face_vertices[0], face_vertices[i - 1], face_vertices[i] }))
					return false;

			num_face_vertices = 0;
			return true;
		}

		bool consumeObjectName(OBJ::Stream& stream, std::string_view name) noexcept
		{
			return true;
		}

		bool consumeGroupName(OBJ::Stream& stream, std::string_view name) noexcept
		{
			return true;
		}

		bool finishGroupAssignment(OBJ::Stream& streame) noexcept
		{
			return true;
		}

		[[nodiscard]]
		bool consumeSmoothingGroup(OBJ::Stream& stream, int n) noexcept
		{
			stream.warn("smoothing groups are ignored!");
			return true;
		}

		[[nodiscard]]
		bool consumeMtlLib(OBJ::Stream& stream, std::string_view name) noexcept
		{
			stream.warn("materials are ignored!");
			return true;
		}

		[[nodiscard]]
		bool consumeUseMtl(OBJ::Stream& stream, std::string_view name) noexcept
		{
			stream.warn("materials are ignored!");
			return true;
		}

		OBJ::Triangles finish() noexcept
		{
			return { std::move(positions), std::move(normals), std::move(texcoords), std::move(triangles) };
		}
	};

	const char* getFileName(const char* path) noexcept
	{
		auto len = std::strlen(path);

		auto beg = std::find_if(std::make_reverse_iterator(path), std::make_reverse_iterator(path + len), [](auto c)
		{
			return c == '/' || c == '\\';
		});

		return beg.base();
	}

	struct Buffer
	{
		std::unique_ptr<char[]> data;
		std::size_t size;
	};

	OBJ::error readFile(Buffer& out, const char* path) noexcept
	{
		struct fcloseDeleter
		{
			void operator ()(FILE* file) const
			{
				if (fclose(file) != 0)
					abort();
			}
		};

		auto file = std::unique_ptr<std::FILE, fcloseDeleter> { std::fopen(path, "rb") };

		if (!file)
			return OBJ::error::FAILED_TO_OPEN_FILE;

		if (!fseek(file.get(), 0, SEEK_END))
			return OBJ::error::FAILED_TO_READ_FILE;

		auto size = ftell(file.get());

		if (size == -1L)
			return OBJ::error::FAILED_TO_READ_FILE;

		if (!fseek(file.get(), 0, SEEK_SET))
			return OBJ::error::FAILED_TO_READ_FILE;

		auto data = std::unique_ptr<char[]>{ new char[size] };

		if (fread(&data[0], 1, size, file.get()) != size)
			return OBJ::error::FAILED_TO_READ_FILE;

		out.data = std::move(data);
		out.size = size;
		return OBJ::error::SUCCESS;
	}
}

namespace OBJ
{
	error readTriangles(Triangles& out, const char* begin, const char* end, const char* name, StreamCallback& stream_callback) noexcept
	{
		Stream stream(begin, end, name, stream_callback);
		OBJConsumer consumer;
		Reader<OBJConsumer> reader(consumer);
		if (error err = stream.consume(reader); err != error::SUCCESS)
			return err;
		out = consumer.finish();
		return error::SUCCESS;
	}

	error readTrianglesFromFile(Triangles& out, const char* path, StreamCallback& stream_callback) noexcept
	{
		Buffer buffer;
		if (error err = readFile(buffer, path); err != error::SUCCESS)
			return err;
		return readTriangles(out, &buffer.data[0], &buffer.data[0] + buffer.size, getFileName(path), stream_callback);
	}

	const char* describeError(error e) noexcept
	{
		switch (e)
		{
		case error::SUCCESS:
			return "success";

		case error::FAILED_TO_OPEN_FILE:
			return "failed to open obj file";

		case error::FAILED_TO_READ_FILE:
			return "failed to read obj file";

		case error::SYNTAX_ERROR:
			return "syntax error";

		case error::UNSUPPORTED_FEATURE:
			return "unsupported feature";
		}

		return "unknown error code";
	}
}
