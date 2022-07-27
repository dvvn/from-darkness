module;

#include "ctype.h"

#include <algorithm>
#include <array>
#include <limits>

export module fd.ctype;
export import fd.string;
import fd.functional;

template <class ArT, class AcT>
class ctype_to
{
    [[no_unique_address]] ArT adaptor_rt_;
    [[no_unique_address]] AcT adaptor_ct_;

    template <typename T>
    constexpr bool process(const T chr) const
    {
        static_assert(!std::is_class_v<T>);
        if constexpr (fd::invocable<AcT, T>)
        {
            if (std::is_constant_evaluated())
                return fd::invoke(adaptor_ct_, chr);
        }
        return fd::invoke(adaptor_rt_, chr);
    }

  public:
    constexpr ctype_to() = default;

    template <typename T>
    constexpr auto operator()(const T& item) const
    {
        if constexpr (std::is_class_v<T>)
        {
            fd::basic_string<std::iter_value_t<T>> buff;
            buff.reserve(item.size());
            for (const auto chr : item)
                buff += process(chr);
            return buff;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            using val_t = std::iter_value_t<T>;
            fd::basic_string<val_t> buff;
            for (auto ptr = item;; ++ptr)
            {
                const auto chr = *ptr;
                if (chr == static_cast<val_t>('\0'))
                    break;
                buff += process(chr);
            }
            return buff;
        }
        else if constexpr (std::is_bounded_array_v<T>)
        {
            const auto size  = std::size(item) - 1;
            const auto begin = item;
            const auto end   = item + size;
            fd::basic_string<std::iter_value_t<T>> buff;
            buff.reserve(size);
            for (auto itr = begin; itr != end; ++itr)
                buff += process(*itr);
            return buff;
        }
        else
        {
            return process(item);
        }
    }
};

/* template <class A>
ctype_to(A&&) -> ctype_to<std::remove_cvref_t<A>>; */

struct is_any_t
{
};

struct is_all_t
{
};

template <class ArT, class AcT>
class ctype_is
{
    [[no_unique_address]] ArT adaptor_rt_;
    [[no_unique_address]] AcT adaptor_ct_;

    template <typename T>
    constexpr bool process(const T chr) const
    {
        static_assert(!std::is_class_v<T>);
        if constexpr (fd::invocable<AcT, T>)
        {
            if (std::is_constant_evaluated())
                return fd::invoke(adaptor_ct_, chr);
        }
        return fd::invoke(adaptor_rt_, chr);
    }

    template <typename T>
    constexpr bool process(const T* ptr, const is_any_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (process(chr))
                return true;
        }
        return false;
    }

    template <typename T>
    constexpr bool process(const T* ptr, const is_all_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (!process(chr))
                return false;
        }
        return true;
    }

    template <typename C>
    constexpr bool process(const fd::basic_string_view<C> strv, const is_any_t) const
    {
        for (const auto chr : strv)
        {
            if (process(chr))
                return true;
        }
        return false;
    }

    template <typename C>
    constexpr bool process(const fd::basic_string_view<C> strv, const is_all_t) const
    {
        for (const auto chr : strv)
        {
            if (!process(chr))
                return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool process(const T obj, const void*) const
    {
        // default
        return process(obj, is_all_t());
    }

  public:
    constexpr ctype_is() = default;

    template <typename T, class M = void*>
    constexpr auto operator()(const T& item, const M mode = {}) const
    {
        if constexpr (std::is_class_v<T>)
        {
            return process(fd::basic_string_view(item), mode);
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            return process(item, mode);
        }
        else if constexpr (std::is_bounded_array_v<T>)
        {
            return process(fd::basic_string_view(item, std::size(item) - 1), mode);
        }
        else
        {
            static_assert(std::is_same_v<M, void*>);
            return process(item);
        }
    }
};

/* template <class A>
ctype_is(A&&) -> ctype_is<std::remove_cvref_t<A>>; */

template <char From, char To>
consteval auto _Make_chars_range()
{
    static_assert(From < To);
    std::array<char, To - From + 1> buff;
    for (size_t i = 0; i < buff.size(); ++i)
        buff[i] = static_cast<char>(From + i);
    return buff;
}

constexpr auto upper_chars  = _Make_chars_range<'A', 'Z'>();
constexpr auto lower_chars  = _Make_chars_range<'a', 'z'>();
constexpr auto number_chars = _Make_chars_range<'0', '9'>();

template <typename T>
constexpr char _Get_char_safe(const T chr)
{
    if constexpr (std::same_as<T, wchar_t>)
    {
        const auto c  = static_cast<char>(chr);
        const auto wc = static_cast<wchar_t>(c);
        if (wc != chr)
            throw;
    }
    return static_cast<char>(chr);
}

CTYPE_IFC(to, lower)
{
    const auto c   = _Get_char_safe(chr);
    const auto bg  = upper_chars.begin();
    const auto end = upper_chars.end();
    const auto pos = std::find(bg, end, c);
    return pos == end ? c : lower_chars[std::distance(bg, pos)];
}

CTYPE_IFC(to, upper)
{
    const auto c   = _Get_char_safe(chr);
    const auto bg  = lower_chars.begin();
    const auto end = lower_chars.end();
    const auto pos = std::find(bg, end, c);
    return pos == end ? c : upper_chars[std::distance(bg, pos)];
}

CTYPE_IFC(is, alnum)
{
    throw; // WIP
}

CTYPE_IFC(is, lower)
{
    throw; // WIP
}

CTYPE_IFC(is, upper)
{
    throw; // WIP
}

CTYPE_IFC(is, digit)
{
    const auto c   = _Get_char_safe(chr);
    const auto bg  = number_chars.begin();
    const auto end = number_chars.end();
    return std::find(bg, end, c) != end;
}

CTYPE_IFC(is, xdigit)
{
    throw; // WIP
}

CTYPE_IFC(is, cntrl)
{
    throw; // WIP
}

CTYPE_IFC(is, graph)
{
    throw; // WIP
}

CTYPE_IFC(is, space)
{
    throw; // WIP
}

CTYPE_IFC(is, print)
{
    throw; // WIP
}

CTYPE_IFC(is, punct)
{
    throw; // WIP
}

export namespace fd
{
    constexpr is_any_t is_any;
    constexpr is_all_t is_all;
} // namespace fd
