#pragma once

#include <fd/hook.h>

#include <tuple>
#include <vector>

namespace fd
{
class hooks_storage final
{
    std::vector<basic_hook*> hooks_;

  public:
    void store(basic_hook* hook);
    void store(basic_hook& hook);

    bool enable();
    bool disable();
};

template <class H>
static bool _enable_hook(H& hook)
{
    return hook.active() || hook.enable();
}

template <class H>
static bool _disable_hook(H& hook)
{
    return !hook.active() || hook.disable();
}

template <class... H>
class hooks_storage2
{
    union
    {
        // ReSharper disable once CppInconsistentNaming
        std::tuple<H...> hooks_;
    };

  public:
    ~hooks_storage2()
    {
        disable_helper<true>(std::make_index_sequence<sizeof...(H)>());
    }

    hooks_storage2(H&&... hooks)
        : hooks_(std::move(hooks)...)
    {
    }

    bool enable()
    {
        return (_enable_hook(std::get<H>(hooks_)) && ...);
    }

  private:
    template <bool Destroy, size_t... I>
    auto disable_helper(std::index_sequence<I...> seq)
    {
        constexpr auto idx = seq.size() - 1;
        if constexpr (!Destroy)
            return ((_disable_hook(std::get<idx - I>(hooks_))) + ...) == seq.size();
        else
            (std::destroy_at(&std::get<idx - I>(hooks_)), ...);
    }

  public:
    bool disable()
    {
        return disable_helper<false>( std::make_index_sequence<sizeof...(H)>());
    }
};
};