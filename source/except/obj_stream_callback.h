#ifndef INCLUDED_OBJ_STREAM_CALLBACK
#define INCLUDED_OBJ_STREAM_CALLBACK

#pragma once

#include "obj.h"


namespace OBJ
{
	struct StdoutStreamCallback : virtual StreamCallback
	{
		virtual void progress(float progress) override;
		virtual void warning(std::string_view file, int line, std::string_view msg) override;
		virtual void error(std::string_view file, int line, std::string_view msg) override;
		virtual void finish() override;
	};
}

#endif  // INCLUDED_OBJ_STREAM_CALLBACK
