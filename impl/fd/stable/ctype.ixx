module;

#include "ctype.h"

#include <algorithm>
#include <functional>
#include <string>

export module fd.ctype;

template <typename T>
concept have_allocator = requires(const T& s)
{
    s.get_allocator();
};

template <typename T>
auto get_allocator(const T& s)
{
    if constexpr (have_allocator<T>)
        return s.get_allocator();
    else
        return std::allocator<std::iter_value_t<T>>();
}

template <class A>
class ctype_to
{
    A adaptor_;

  public:
    template <typename A0>
    constexpr ctype_to(A0&& adaptor)
        : adaptor_(std::forward<A0>(adaptor))
    {
    }

    constexpr ctype_to() = default;

    template <typename T>
    auto operator()(const T& item) const
    {
        if constexpr (std::is_class_v<T>)
        {
            using val_t    = std::iter_value_t<T>;
            using traits_t = std::char_traits<val_t>;
            using alloc_t  = decltype(get_allocator(item));
            std::basic_string<val_t, traits_t, alloc_t> buff(get_allocator(item));
            buff.reserve(item.size());
            for (const auto chr : item)
                buff += std::invoke(adaptor_, chr);
            return buff;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            using val_t = std::iter_value_t<T>;
            std::basic_string<val_t> buff;
            for (auto ptr = item;;)
            {
                auto chr = *ptr++;
                if (chr == static_cast<val_t>('\0'))
                    break;
                buff += std::invoke(adaptor_, chr);
            }
            return buff;
        }
        else if constexpr (std::is_bounded_array_v<T>)
        {
            const auto size  = std::size(item) - 1;
            const auto begin = item;
            const auto end   = item + size;
            std::basic_string<std::iter_value_t<T>> buff;
            buff.reserve(size);
            for (auto itr = begin; itr != end; ++itr)
                buff += std::invoke(adaptor_, *itr);
            return buff;
        }
        else
        {
            return std::invoke(adaptor_, item);
        }
    }
};

template <class A>
ctype_to(A&&) -> ctype_to<std::remove_cvref_t<A>>;

struct is_any_t
{
};

struct is_all_t
{
};

template <class A>
class ctype_is
{
    A adaptor_;

    template <typename T>
    bool process(const T chr) const
    {
        static_assert(!std::is_class_v<T>);
        return std::invoke(adaptor_, chr);
    }

    template <typename T>
    bool process(const T* ptr, const is_any_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (process(chr))
                return true;
        }
        return false;
    }

    template <typename T>
    bool process(const T* ptr, const is_all_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (!process(chr))
                return false;
        }
        return true;
    }

    template <typename... S>
    bool process(const std::basic_string_view<S...> strv, const is_any_t) const
    {
        for (const auto chr : strv)
        {
            if (process(chr))
                return true;
        }
        return false;
    }

    template <typename... S>
    bool process(const std::basic_string_view<S...> strv, const is_all_t) const
    {
        for (const auto chr : strv)
        {
            if (!process(chr))
                return false;
        }
        return true;
    }

    template <typename T>
    bool process(const T obj, const void*) const
    {
        // default
        return process(obj, is_all_t());
    }

  public:
    template <typename A0>
    constexpr ctype_is(A0&& adaptor)
        : adaptor_(std::forward<A0>(adaptor))
    {
    }

    constexpr ctype_is() = default;

    template <typename T, class M = void*>
    auto operator()(const T& item, const M mode = {}) const
    {
        if constexpr (std::is_class_v<T>)
        {
            return process(std::basic_string_view(item), mode);
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            return process(item, mode);
        }
        else if constexpr (std::is_bounded_array_v<T>)
        {
            return process(std::basic_string_view(item, std::size(item) - 1), mode);
        }
        else
        {
            static_assert(std::is_same_v<M, void*>);
            return process(item);
        }
    }
};

template <class A>
ctype_is(A&&) -> ctype_is<std::remove_cvref_t<A>>;

CTYPE_IFC(to, lower);
CTYPE_IFC(to, upper);
CTYPE_IFC(is, alnum);
CTYPE_IFC(is, lower);
CTYPE_IFC(is, upper);
CTYPE_IFC(is, digit);
CTYPE_IFC(is, xdigit);
CTYPE_IFC(is, cntrl);
CTYPE_IFC(is, graph);
CTYPE_IFC(is, space);
CTYPE_IFC(is, print);
CTYPE_IFC(is, punct);

export namespace fd
{
    constexpr is_any_t is_any;
    constexpr is_all_t is_all;
} // namespace fd