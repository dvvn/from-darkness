#pragma once

#include "basic/winapi.h"
#include "hook/holder.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

struct basic_win32_backend;
class hooked_wndproc;

template <>
struct make_incomplete_object<hooked_wndproc> final
{
    prepared_hook_data_full<basic_winapi_hook*> operator()(basic_win32_backend* backend) const;
};
} // namespace fd