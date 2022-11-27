#pragma once

#include <tuple>

namespace fd
{
    template <typename... H>
    class hook_holder
    {
        union
        {
            std::tuple<H...> hooks_;
        };

        using seq_t = std::index_sequence_for<H...>;

        template <size_t... I>
        bool _Enable(const std::index_sequence<I...>)
        {
            // call directly
            return (std::get<I>(hooks_).enable() && ...);
        }

        template <size_t... I>
        bool _Disable(const std::index_sequence<I...> seq)
        {
            // reverse the call
            constexpr auto last = seq.size() - 1;
            return (std::get<last - I>(hooks_).disable() && ...);
        }

      public:
        ~hook_holder()
        {
            disable();
        }

        hook_holder(H&&... hooks)
            : hooks_(std::move(hooks)...)
        {
        }

        bool enable()
        {
            return _Enable(seq_t());
        }

        bool disable()
        {
            return _Disable(seq_t());
        }
    };
} // namespace fd