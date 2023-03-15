#pragma once

#include <fd/hooking/callback.h>

#include <boost/hana/tuple.hpp>

namespace fd
{
template <typename... Args>
class hook_callback_args final
{
    boost::hana::tuple<Args...> args_;

  public:
    hook_callback_args(Args... args)
        : args_(std::move(args)...)
    {
    }

    auto get() const &
    {
        return boost::hana::unpack(args_, [](Args const &...args) { return hook_callback(args...); });
    }

    auto get() &&
    {
        return boost::hana::unpack(args_, [](Args &...args) { return hook_callback(std::move(args)...); });
    }
};

static constexpr struct
{
    template <typename Callback, _x86_call Call, typename... Args>
    auto operator()(hook_callback<Callback, Call, Args...> &&cb) const
    {
        return std::move(cb);
    }

    template <typename... T>
    auto operator()(hook_callback_args<T...> &&cb) const
    {
        return std::move(cb).get();
    }
} _FwdHookCallback;

template <class Tpl, class ArgsTpl, size_t Idx, size_t... I>
static void _hooks_storage_init(Tpl &tpl, ArgsTpl &args, std::index_sequence<Idx, I...>)
{
    std::construct_at(&boost::hana::at_c<Idx>(tpl), _FwdHookCallback(std::move(boost::hana::at_c<Idx>(args))));
    if constexpr (sizeof...(I) != 0)
        _hooks_storage_init(tpl, args, std::index_sequence<I...>());
}

template <class Tpl, class ArgsTpl, size_t... I>
static Tpl _hooks_storage_init(ArgsTpl args, std::index_sequence<I...> seq)
{
    uint8_t buff[sizeof(Tpl)];
    auto &tpl = reinterpret_cast<Tpl &>(buff);

    _hooks_storage_init(tpl, args, seq);

    auto ret = std::move(tpl);
    std::destroy_at(&tpl);
    return ret;
}

template <class... H>
class hooks_storage final
{
    using storage_type = boost::hana::tuple<H...>;

    storage_type hooks_;

    //_hooks_storage_init used to force left-to-right args initializtion order

  public:
    template <typename... Args>
    hooks_storage(Args &&...args)
        : hooks_(_hooks_storage_init<storage_type>(
              boost::hana::tuple<Args &&...>(std::forward<Args>(args)...),
              std::index_sequence_for<H...>()))
    {
    }

    bool enable()
    {
        return boost::hana::unpack(hooks_, [](H &...h) { return (h.enable() && ...); });
    }

    bool disable()
    {
        return [&]<size_t... I>(std::index_sequence<I...> seq) -> bool {
            return (boost::hana::at_c<seq.size() - I - 1>(hooks_).disable() & ...);
        }(std::make_index_sequence<sizeof...(H)>());
    }
};

template <class... H>
hooks_storage(H... hooks) -> hooks_storage<std::decay_t<decltype(_FwdHookCallback(static_cast<H &&>(hooks)))>...>;
} // namespace fd