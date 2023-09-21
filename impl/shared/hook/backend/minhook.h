#pragma once

#include "hook/basic_backend.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

struct backend_minhook;

template <>
struct make_incomplete_object<backend_minhook> final
{
    basic_hook_backend* operator()() const;
};

} // namespace fd