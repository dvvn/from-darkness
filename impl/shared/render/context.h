#pragma once

#include "basic_context.h"

namespace fd
{
class render_context;

template <class T>
struct make_incomplete_object;

template <>
struct make_incomplete_object<render_context>final
{
    basic_render_context*operator()()const;
};

} // namespace fd