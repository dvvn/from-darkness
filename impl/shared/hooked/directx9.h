#pragma once
#include "basic/directx9.h"
#include "hook/holder.h"

namespace fd
{
template <class T>
struct make_incomplete_object;

struct basic_dx9_backend;
class hooked_directx9_reset;

template <>
struct make_incomplete_object<hooked_directx9_reset> final
{
    prepared_hook_data_full<basic_directx9_hook*> operator()(basic_dx9_backend* backend) const;
};

struct render_frame;
class hooked_directx9_present;

template <>
struct make_incomplete_object<hooked_directx9_present> final
{
    prepared_hook_data_full<basic_directx9_hook*> operator()(render_frame const* render) const;
};
} // namespace fd