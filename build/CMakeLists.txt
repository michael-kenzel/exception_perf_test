cmake_minimum_required(VERSION 3.11)

project(exception_perf_test)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../source")

add_library(math INTERFACE)

set_target_properties(math PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}"
)

add_executable(except
	"${SOURCE_DIR}/except/obj_stream.h"
	"${SOURCE_DIR}/except/obj_reader.h"
	"${SOURCE_DIR}/except/obj_stream_callback.h"
	"${SOURCE_DIR}/except/obj_stream_callback.cpp"
	"${SOURCE_DIR}/except/obj.h"
	"${SOURCE_DIR}/except/obj.cpp"
	"${SOURCE_DIR}/except/obj_reader.h"
	"${SOURCE_DIR}/except/obj_stream.h"
	"${SOURCE_DIR}/except/main.cpp"
)

target_link_libraries(except math)

add_executable(noexcept
	"${SOURCE_DIR}/noexcept/dynamic_array.h"
	"${SOURCE_DIR}/noexcept/hash_map.h"
	"${SOURCE_DIR}/noexcept/obj_stream.h"
	"${SOURCE_DIR}/noexcept/obj_reader.h"
	"${SOURCE_DIR}/noexcept/obj_stream_callback.h"
	"${SOURCE_DIR}/noexcept/obj_stream_callback.cpp"
	"${SOURCE_DIR}/noexcept/obj.h"
	"${SOURCE_DIR}/noexcept/obj.cpp"
	"${SOURCE_DIR}/noexcept/obj_reader.h"
	"${SOURCE_DIR}/noexcept/obj_stream.h"
	"${SOURCE_DIR}/noexcept/main.cpp"
)

target_link_libraries(noexcept math)

source_group(source ".*\.((h$)|(cpp$))")

set_target_properties(except noexcept PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

if (MSVC)
	target_compile_options(except PRIVATE /WX /MP /Gm- /permissive-)
	target_compile_options(noexcept PRIVATE /WX /MP /Gm- /permissive- /EHs-c-)
	target_compile_definitions(except PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
	target_compile_definitions(noexcept PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
else ()
	target_compile_options(noexcept PRIVATE -fno-exceptions)
endif ()
