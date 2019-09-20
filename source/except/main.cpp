#include <stdexcept>
#include <iterator>
#include <iostream>

#include "obj_stream_callback.h"
#include "obj.h"


namespace
{
	struct usage_error : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	std::ostream& printUsage(std::ostream& out)
	{
		return out << "objstat <filename>";
	}
}

int main(int argc, const char* argv[])
{
	try
	{
		if (argc < 2)
			throw usage_error("expected <filename>");
		else if (argc > 2)
			throw usage_error("too many arguments");

		OBJ::StdoutStreamCallback callback;
		auto obj = OBJ::readTriangles(argv[1], callback);

		std::cout << size(obj.positions) << " positions, " << size(obj.normals) << " normals, " << size(obj.texcoords) << " texcoords, " << size(obj.triangles) << " triangles\n";
	}
	catch (const usage_error & e)
	{
		printUsage(std::cerr << "error: " << e.what() << '\n');
		return -2;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}
	catch (...)
	{
		std::cerr << "error: unknown exception\n";
		return -128;
	}

	return 0;
}
