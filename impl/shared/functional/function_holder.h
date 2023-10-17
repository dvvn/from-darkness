#pragma once

#include "functional/basic_function.h"
#include "functional/call_traits.h"

namespace fd
{
template <
    typename Fn,                                            //
    typename Ret = typename function_info<Fn>::return_type, //
    class Args   = typename function_info<Fn>::args>
class function_holder;

template <typename Fn, typename Ret, class... Args>
class function_holder<Fn, Ret, function_args<Args...>> final : public basic_function<Ret, Args...>
{
    Fn fn_;

  public:
    constexpr function_holder(Fn&& fn, std::in_place_type_t<Ret> = std::in_place_type<Ret>)
        : fn_(std::move(fn))
    {
    }

    constexpr function_holder(Fn const& fn, std::in_place_type_t<Ret> = std::in_place_type<Ret>)
        : fn_((fn))
    {
    }

    Ret operator()(Args... args) const override
    {
        using real_ret = typename function_info<Fn>::return_type;
        if constexpr (std::same_as<Ret, real_ret> || !std::is_void_v<Ret>)
            return fn_(static_cast<Args>(args)...);
        else
            return (void)fn_(static_cast<Args>(args)...);
    }
};

template <typename Fn>
function_holder(Fn) -> function_holder<std::decay_t<Fn>>;

template <typename Fn, typename Ret>
function_holder(Fn, std::in_place_type_t<Ret>) -> function_holder<std::decay_t<Fn>, Ret>;

} // namespace fd