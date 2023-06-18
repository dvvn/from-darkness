#pragma once

#include "core.h"

#include <mutex>

namespace fd
{
FD_WRAP_TOOL_SIMPLE(mutex, std::mutex);

#ifdef _DEBUG
template <class>
struct lock_guard;

template <>
FD_WRAP_TOOL_SIMPLE(lock_guard<mutex>, std::lock_guard<std::mutex>);

template <typename T>
lock_guard(T &) -> lock_guard<T>;
#else
using std::lock_guard;
#endif
}