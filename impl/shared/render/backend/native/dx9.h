#pragma once
#include "render/backend/basic_dx9.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

class system_library_info;
class native_dx9_backend;

template <>
struct make_incomplete_object<native_dx9_backend> final
{
    basic_dx9_backend* operator()(system_library_info info) const;
};
} // namespace fd