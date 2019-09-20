#include <cstdio>

#include "obj_stream_callback.h"


namespace OBJ
{
	void StdoutStreamCallback::progress(float p) noexcept
	{
		printf("\r%f.0", 100.0f * p);
		fflush(stdout);
	}

	void StdoutStreamCallback::warning(const char* file, int line, const char* msg) noexcept
	{
		fprintf(stderr, "%s(%d): error: %s\n", file, line, msg);
	}

	void StdoutStreamCallback::error(const char* file, int line, const char* msg) noexcept
	{
		fprintf(stderr, "%s(%d): warning: %s\n", file, line, msg);
	}

	void StdoutStreamCallback::finish() noexcept
	{
		puts("\r100%\n");
		fflush(stdout);
	}
}
