#include <cstdio>
#include <iterator>

#include "obj_stream_callback.h"
#include "obj.h"


namespace
{
	void printUsage()
	{
		puts("objstat <filename>");
	}
}

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		puts("error: expected <filename>\n");
		printUsage();
		return -2;
	}
	else if (argc > 2)
	{
		puts("error: too many arguments\n");
		printUsage();
		return -1;
	}

	OBJ::StdoutStreamCallback callback;
	OBJ::Triangles obj;
	if (auto err = OBJ::readTrianglesFromFile(obj, argv[1], callback); err != OBJ::error::SUCCESS)
	{
		printf("error: %s", OBJ::describeError(err));
		return -1;
	}

	obj.triangles.push_back({});

	printf("%zu positions, %zu normals, %zu texcoords, %zu triangles\n", size(obj.positions), size(obj.normals), size(obj.texcoords), size(obj.triangles));

	return 0;
}
