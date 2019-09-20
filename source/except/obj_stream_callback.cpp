#include <iostream>
#include <iomanip>

#include "obj_stream_callback.h"


namespace OBJ
{
	void StdoutStreamCallback::progress(float p)
	{
		std::cout << '\r' << std::fixed << std::setprecision(0) << 100.0f * p << '%' << std::flush;
	}

	void StdoutStreamCallback::warning(std::string_view file, int line, std::string_view msg)
	{
		std::cerr << file << '(' << line << "): error: " << msg;
	}

	void StdoutStreamCallback::error(std::string_view file, int line, std::string_view msg)
	{
		std::cerr << file << '(' << line << "): warning: " << msg;
	}

	void StdoutStreamCallback::finish()
	{
		std::cout << "\r100%\n" << std::flush;
	}
}
