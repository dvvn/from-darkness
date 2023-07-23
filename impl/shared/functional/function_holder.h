#pragma once

#include "basic_function.h"
#include "call_traits.h"

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

  protected:
    ~function_holder() = default;

  public:
    constexpr function_holder(Fn fn)
        : fn_(std::move(fn))
    {
    }

    Ret operator()(Args... args) const override
    {
        return fn_(static_cast<Args>(args)...);
    }
};

template <typename Fn>
function_holder(Fn) -> function_holder<std::decay_t<Fn>>;

} // namespace fd