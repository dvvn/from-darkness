#pragma once

#include "basic_search_stop_token.h"
#include "functional/call_traits.h"

namespace fd
{
template <typename Fn>
class search_stop_token final : public basic_search_stop_token
{
    using arg_t = typename function_info<Fn>::template arg<0>;

    Fn fn_;

  public:
    search_stop_token(Fn fn)
        : fn_(std::move(fn))
    {
        static_assert(!std::is_reference_v<arg_t>);
    }

    bool operator()(void *found) const override
    {
        return fn_(unsafe_cast<arg_t /*, void *&*/>(found));
    }
};

} // namespace fd