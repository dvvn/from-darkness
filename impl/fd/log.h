#pragma once

#include <fd/string.h>

namespace fd
{
void log(string_view msg);
void log(wstring_view msg);

#ifdef _DEBUG
// ReSharper disable once CppVariableCanBeMadeConstexpr
inline const bool _log_active = true; //added to prevent if constexpr hints
// ReSharper disable CppInconsistentNaming
#define log_active() _log_active
#define log_unsafe   log
// ReSharper restore CppInconsistentNaming
#else
bool log_active();
void log_unsafe(string_view msg);
void log_unsafe(wstring_view msg);
#endif
}