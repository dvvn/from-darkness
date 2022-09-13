module;

#if __has_include(<veque.hpp>)
#include <veque.hpp>
#else
#include <deque>
#endif

#include <vector>

export module fd.callback;
export import fd.callback.base;
export import fd.functional.fn;

#ifdef VEQUE_HEADER_GUARD
#define CALLBACK_STORAGE veque::veque
#else
#define CALLBACK_STORAGE std::deque
#endif

export namespace fd
{
    template <typename Fn>
    using callback_custom = basic_callback<CALLBACK_STORAGE, Fn>;

    template <typename... Args>
    using callback = basic_callback<CALLBACK_STORAGE, function<void(Args...) const>>;

    template <typename Fn>
    using callback_simple_custom = basic_callback<std::vector, Fn>;

    template <typename... Args>
    using callback_simple = basic_callback<std::vector, function<void(Args...) const>>;
} // namespace fd
