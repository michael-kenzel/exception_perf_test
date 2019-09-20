#ifndef INCLUDED_OBJ_STREAM_CALLBACK
#define INCLUDED_OBJ_STREAM_CALLBACK

#pragma once

#include "obj.h"


namespace OBJ
{
	struct StdoutStreamCallback : virtual StreamCallback
	{
		virtual void progress(float progress) noexcept override;
		virtual void warning(const char* file, int line, const char* msg) noexcept override;
		virtual void error(const char* file, int line, const char* msg) noexcept override;
		virtual void finish() noexcept override;
	};
}

#endif  // INCLUDED_OBJ_STREAM_CALLBACK
