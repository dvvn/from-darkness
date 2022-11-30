#pragma once

#include <tuple>

namespace fd
{
    template <typename... H>
    class hook_holder
    {
        union
        {
            // ReSharper disable once CppInconsistentNaming
            std::tuple<H...> hooks_;
        };

        using seq_t = std::index_sequence_for<H...>;

        template <size_t... I>
        bool enable_impl(const std::index_sequence<I...>)
        {
            // call directly
            return (std::get<I>(hooks_).enable() && ...);
        }

        template <size_t... I>
        bool disable_impl(const std::index_sequence<I...> seq)
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
            return enable_impl(seq_t());
        }

        bool disable()
        {
            return disable_impl(seq_t());
        }
    };
} // namespace fd