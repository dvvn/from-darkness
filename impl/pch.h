#pragma once

// ReSharper disable CppInconsistentNaming

#define NOMINMAX
#define WIN32
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
//#define _CRT_SECURE_NO_WARNINGS
#define _HAS_DEPRECATED_ALLOCATOR_MEMBERS 0
#define _FSTREAM_SUPPORTS_EXPERIMENTAL_FILESYSTEM 0

#include "runtime assert.h"

#define _ASSERT runtime_assert
#define _ASSERTE runtime_assert
#define _ASSERT_EXPR runtime_assert
#include <crtdbg.h>

#include <concrt.h>
#undef _CONCRT_ASSERT
#define _CONCRT_ASSERT runtime_assert

#include <yvals.h>
#undef _STL_REPORT_ERROR
#define _STL_REPORT_ERROR runtime_assert
#undef _STL_VERIFY
#define _STL_VERIFY runtime_assert

//-----

#include <d3d9.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <Windows.h>

#include <any>
#include <filesystem>
#include <format>
#include <fstream>
#include <fstream>
#include <functional>
#include <iostream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <semaphore>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <future>
#include <variant>
#include <vector>
#include <queue>

#if defined(__cpp_lib_concepts)
#include <concepts>
#include <format>
#include <ranges>

namespace ranges
{
	namespace views = std::views;
	using namespace std::ranges;
}

#else

#define FMT_ASSERT runtime_assert
#include <fmt/format.h>
namespace std
{
	using fmt::format;
	using fmt::vformat;
}

#define RANGES_ASSERT runtime_assert
#define RANGES_ENSURE_MSG runtime_assert
#include <range/v3/all.hpp>

#endif

#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define TSL_DEBUG
#endif
#include <ordered map/include/tsl/ordered_map.h>
#include <ordered map/include/tsl/ordered_set.h>
#include <robin map/include/tsl/robin_map.h>
#include <robin map/include/tsl/robin_set.h>

#include <include/veque.hpp>

//#define FT2_BUILD_LIBRARY

#if !defined(_DEBUG) && !defined(CHEAT_GUI_TEST)
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1
#define IMGUI_USE_WCHAR32
#define IMGUI_DEBUG_PARANOID
#define IMGUI_DEFINE_MATH_OPERATORS
//#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#define IM_ASSERT runtime_assert
#define IM_ASSERT_USER_ERROR runtime_assert
#define ImDrawIdx size_t
// ReSharper disable CppWrongIncludesOrder
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <misc/cpp/imgui_stdlib.h>
// ReSharper restore CppWrongIncludesOrder

//#include <WinReg/WinReg.hpp>

#include <thread_pool.hpp>

#include "not std.h"

//using namespace std::literals;

#define CHEAT_UNUSED_ASSERT\
	runtime_assert("This function added only as example");\
	(void)this;

#define CHEAT_OUTPUT_DIR NSTD_RAW(VS_OutputDir)"\\"
#define CHEAT_SOLUTION_DIR NSTD_RAW(VS_SolutionDir)"\\"
#define CHEAT_NAME _STRINGIZE(VS_SolutionName)
#define CHEAT_DUMPS_DIR /*CHEAT_OUTPUT_DIR*/CHEAT_SOLUTION_DIR NSTD_RAW(.out\dumps\)
#define CHEAT_IMPL_DIR CHEAT_SOLUTION_DIR NSTD_RAW(impl\cheat\)

#define CHEAT_CURRENT_FILE_PATH\
	std::string_view(##__FILE__).substr(std::string_view(CHEAT_SOLUTION_DIR).size( ) + /*impl\*/5)

#include "cheat/utils/memory block.h"
//dont sort
#include "cheat/utils/memory backup.h"
#include "cheat/utils/memory protect.h"
#include "cheat/utils/module info/module info.h"
#include "cheat/utils/winapi/threads.h"

#include "cheat/hooks/_impl/hook_utils.h"

#include "cheat/sdk/Color.hpp"
#include "cheat/sdk/QAngle.hpp"
#include "cheat/sdk/Vector.hpp"
#include "cheat/sdk/Vector2D.hpp"
#include "cheat/sdk/Vector4D.hpp"
#include "cheat/sdk/VMatrix.hpp"