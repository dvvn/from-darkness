#pragma once

#include "functional/call_traits.h"
#include "functional/function_holder.h"

namespace fd
{
template <typename Fn>
auto make_search_stop_token(Fn fn)
{
    using arg_t = typename function_info<Fn>::template arg<0>;
    static_assert(!std::is_reference_v<arg_t>);

    return function_holder([fn = std::move(fn)](void *found) -> bool {
        return fn(unsafe_cast<arg_t /*, void *&*/>(found));
    });
}
} // namespace fd