module;

#include <cctype>
#include <cwctype>

#include <algorithm>
#include <array>
#include <limits>
#include <span>

export module fd.string.info;
export import fd.string;
import fd.functional.invoke;

using namespace fd;

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
            basic_string<std::iter_value_t<T>> buff;
            buff.reserve(item.size());
            for (auto c : item)
                buff += invoke(*this, c);
            return buff;
        }
        else
        {
            if constexpr (invocable<AcT, T>)
            {
                if (std::is_constant_evaluated())
                    return invoke(adaptor_ct_, item);
            }
            return invoke(adaptor_rt_, item);
        }
    }

    template <typename T>
    constexpr auto operator()(const T* item) const
    {
        return invoke(*this, std::span(item, str_len(item)));
    }

    template <typename T>
    constexpr auto operator()(const T from, const size_t size) const
    {
        return invoke(*this, std::span(from, size));
    }

    template <typename T>
    constexpr auto operator()(const T from, const T to) const
    {
        return invoke(*this, std::span(from, to));
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
            return invoke(*this, item, default_mode_t());
        }
        else
        {
            if constexpr (invocable<AcT, T>)
            {
                if (std::is_constant_evaluated())
                    return invoke(adaptor_ct_, item);
            }
            return invoke(adaptor_rt_, item);
        }
    }

    template <class T>
    constexpr bool operator()(const T& rng, const is_any_t) const
    {
        for (auto c : rng)
        {
            if (invoke(*this, c))
                return true;
        }
        return false;
    }

    template <class T>
    constexpr bool operator()(const T& rng, const is_all_t) const
    {
        for (auto c : rng)
        {
            if (!invoke(*this, c))
                return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr, const is_any_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (invoke(*this, chr))
                return true;
        }
        return false;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr, const is_all_t) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (!invoke(*this, chr))
                return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr)
    {
        return invoke(*this, ptr, default_mode_t());
    }

    template <typename T, class Mode = default_mode_t>
    constexpr bool operator()(const T from, const size_t size, const Mode mode = {}) const
    {
        return invoke(*this, std::span(from, size), mode);
    }

    template <typename T, class Mode = default_mode_t>
    constexpr bool operator()(const T from, const T to, const Mode mode = {}) const
    {
        return invoke(*this, std::span(from, to), mode);
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

template <size_t... S>
constexpr auto _Joint_arrays(const std::array<char, S>&... src)
{
    std::array<char, (S + ...)> buff;
    auto itr = buff.begin();

    ((itr = std::copy(src.begin(), src.end(), itr)), ...);
    return buff;
}

constexpr auto upper_chars  = _Make_chars_range<'A', 'Z'>();
constexpr auto lower_chars  = _Make_chars_range<'a', 'z'>();
constexpr auto number_chars = _Make_chars_range<'0', '9'>();
constexpr auto hex_chars    = _Joint_arrays(number_chars, _Make_chars_range<'a', 'f'>(), _Make_chars_range<'A', 'F'>());

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

template <typename Ret, typename T, typename Ct, typename Rt>
constexpr Ret _Ct_rt_impl(const T val, Ct ct, Rt rt)
{
    if constexpr (invocable<Ct, T>)
    {
        if (std::is_constant_evaluated())
            return invoke(ct, val);
    }

    return invoke(rt, val);
}

template <typename T, typename... Args>
constexpr T _Ct_rt_to(const T val, Args... fn)
{
    return _Ct_rt_impl<T>(val, fn...);
}

template <typename... Args>
constexpr bool _Ct_rt_is(Args... args)
{
    return _Ct_rt_impl<bool>(args...);
}

constexpr auto _Get_offset = [](const auto val, const auto& buff) -> size_t {
    const auto bg  = buff.begin();
    const auto end = buff.end();
    const auto pos = std::find(bg, end, val);
    return pos == end ? -1 : std::distance(bg, pos);
};

constexpr auto _Get_offset_at = []<typename T>(const T val, const auto& buff, const auto& buff2) -> T {
    const auto offset = _Get_offset(val, buff);
    return offset == -1 ? val : buff2[offset];
};

template <typename T, class Buff>
constexpr bool _Contains(const T val, const Buff& buff)
{
    const auto bg  = buff.begin();
    const auto end = buff.end();
    return std::find(bg, end, val) != end;
}

constexpr auto _To_lower_ct = [](const auto val) {
    return _Get_offset_at(val, upper_chars, lower_chars);
};

struct to_lower_impl
{
    constexpr char operator()(const char val) const
    {
        return _Ct_rt_to(val, _To_lower_ct, std::tolower);
    }

    constexpr wchar_t operator()(const wchar_t val) const
    {
        return _Ct_rt_to(val, _To_lower_ct, std::towlower);
    }
};

constexpr auto _To_upper_ct = [](const auto val) {
    return _Get_offset_at(val, lower_chars, upper_chars);
};

struct to_upper_impl
{
    constexpr char operator()(const char val) const
    {
        return _Ct_rt_to(val, _To_upper_ct, std::toupper);
    }

    constexpr wchar_t operator()(const wchar_t val) const
    {
        return _Ct_rt_to(val, _To_upper_ct, std::towupper);
    }
};

struct is_alnum_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::isalnum);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswalnum);
    }
};

constexpr auto _Is_lower_ct = [](const auto val) {
    return _Contains(val, lower_chars);
};

struct is_lower_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, _Is_lower_ct, std::islower);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, _Is_lower_ct, std::iswlower);
    }
};

constexpr auto _Is_upper_ct = [](const auto val) {
    return _Contains(val, upper_chars);
};

struct is_upper_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, _Is_upper_ct, std::isupper);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, _Is_upper_ct, std::iswupper);
    }
};

struct is_digit_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::isdigit);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswdigit);
    }
};

constexpr auto _Is_xdigit_ct = [](const auto val) {
    return _Contains(val, hex_chars);
};

struct is_xdigit_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, _Is_xdigit_ct, std::isxdigit);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, _Is_xdigit_ct, std::iswxdigit);
    }
};

struct is_cntrl_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::iscntrl);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswcntrl);
    }
};

struct is_graph_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::isgraph);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswgraph);
    }
};

struct is_space_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::isspace);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswspace);
    }
};

struct is_print_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::isprint);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswprint);
    }
};

struct is_punct_impl
{
    constexpr bool operator()(const char val) const
    {
        return _Ct_rt_is(val, nullptr, std::ispunct);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return _Ct_rt_is(val, nullptr, std::iswpunct);
    }
};

export namespace fd
{
    constexpr to_lower_impl to_lower;
    constexpr to_upper_impl to_upper;
    constexpr is_alnum_impl is_alnum;
    constexpr is_lower_impl is_lower;
    constexpr is_upper_impl is_upper;
    constexpr is_digit_impl is_digit;
    constexpr is_xdigit_impl is_xdigit;
    constexpr is_cntrl_impl is_cntrl;
    constexpr is_graph_impl is_graph;
    constexpr is_space_impl is_space;
    constexpr is_print_impl is_print;
    constexpr is_punct_impl is_punct;
} // namespace fd
