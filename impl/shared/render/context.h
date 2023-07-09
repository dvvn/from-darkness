#pragma once

#include "basic_context.h"
#include "interface_holder.h"

namespace fd
{
class render_context;
FD_INTERFACE_FWD(render_context, basic_render_context);

constexpr auto adasdas =
    detail::valid_interface_v<interface_type::heap,
                              construct_interface<interface_type::heap, render_context>::holder_type>;
} // namespace fd