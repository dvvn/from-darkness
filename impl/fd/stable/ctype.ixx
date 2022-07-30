module;

#include "ctype.h"

#include <algorithm>
#include <array>
#include <limits>
#include <ranges>
#include <span>

export module fd.ctype;
export import fd.string;
import fd.functional;

template <class ArT, class AcT>
class ctype_to
{
    [[no_unique_address]] ArT adaptor_rt_;
    [[no_unique_address]] AcT adaptor_ct_;

  public:
    constexpr ctype_to() = default;

    template <typename T>
    constexpr auto operator()(const T& item) const
    {
        if constexpr (std::is_class_v<T>)
        {
            fd::basic_string<std::iter_value_t<T>> buff;
            buff.reserve(item.size());
            for (auto c : item)
                buff += fd::invoke(*this, c);
            return buff;
        }
        else
        {
            if constexpr (fd::invocable<AcT, T>)
            {
                if (std::is_constant_evaluated())
                    return fd::invoke(adaptor_ct_, item);
            }
            return fd::invoke(adaptor_rt_, item);
        }
    }

    template <typename T>
    constexpr auto operator()(const T* item) const
    {
        return fd::invoke(*this, std::span(item, fd::str_len(item)));
    }

    template <typename T>
    constexpr auto operator()(const T from, const size_t size) const
    {
        return fd::invoke(*this, std::span(from, size));
    }

    template <typename T>
    constexpr auto operator()(const T from, const T to) const
    {
        return fd::invoke(*this, std::span(from, to));
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

  public:
    constexpr ctype_is() = default;

    using default_mode_t = is_all_t;

    template <typename T>
    constexpr bool operator()(const T& item) const
    {
        if constexpr (std::is_class_v<T>)
        {
            // run default mode (rng)
            return fd::invoke(*this, item, default_mode_t());
        }
        else
        {
            if constexpr (fd::invocable<AcT, T>)
            {
                if (std::is_constant_evaluated())
                    return fd::invoke(adaptor_ct_, item);
            }
            return fd::invoke(adaptor_rt_, item);
        }
    }

    template <class T>
    constexpr bool operator()(const T& rng, const is_any_t) const
    {
        for (auto c : rng)
        {
            if (fd::invoke(*this, c))
                return true;
        }
        return false;
    }

    template <class T>
    constexpr bool operator()(const T& rng, const is_all_t) const
    {
        for (auto c : rng)
        {
            if (!fd::invoke(*this, c))
                return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr, const is_any_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (fd::invoke(*this, chr))
                return true;
        }
        return false;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr, const is_all_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (!fd::invoke(*this, chr))
                return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr)
    {
        return fd::invoke(*this, ptr, default_mode_t());
    }

    template <typename T, class Mode = default_mode_t>
    constexpr bool operator()(const T from, const size_t size, const Mode mode = {}) const
    {
        return fd::invoke(*this, std::span(from, size), mode);
    }

    template <typename T, class Mode = default_mode_t>
    constexpr bool operator()(const T from, const T to, const Mode mode = {}) const
    {
        return fd::invoke(*this, std::span(from, to), mode);
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
