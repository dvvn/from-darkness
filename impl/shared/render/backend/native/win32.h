#pragma once
#include "render/backend/basic_win32.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

class native_win32_backend;

template <>
struct make_incomplete_object<native_win32_backend> final
{
    basic_win32_backend* operator()() const;
};

} // namespace fd