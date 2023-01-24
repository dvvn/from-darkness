// ReSharper disable CppUnreachableCode
#pragma once

#define _CTYPE_DISABLE_MACROS

#include <fd/functional.h>

#include <cctype>
#include <cwctype>

#include <algorithm>
#include <array>
#include <span>
#include <vector>

namespace fd
{
template <class ArT, class AcT>
class ctype_to
{
    [[no_unique_address]] ArT adaptorRt_;
    [[no_unique_address]] AcT adaptorCt_;

  public:
    constexpr ctype_to() = default;

    template <typename T>
    constexpr auto operator()(const T& item) const
    {
        if constexpr (std::is_class_v<T>)
        {
            std::vector<std::iter_value_t<T>> buff;
            buff.reserve(item.size());
            for (auto c : item)
                buff.push_back(this->operator()(c));
            return buff;
        }
        else
        {
            if constexpr (std::invocable<AcT, T>)
            {
                if (std::is_constant_evaluated())
                    return adaptorCt_(item);
            }
            return adaptorRt_(item);
        }
    }

    template <typename T>
    constexpr auto operator()(const T* item) const
    {
        return this->operator()(std::span(item, str_len(item)));
    }

    template <typename T>
    constexpr auto operator()(const T from, const size_t size) const
    {
        return this->operator()(std::span(from, size));
    }

    template <typename T>
    constexpr auto operator()(const T from, const T to) const
    {
        return this->operator()(std::span(from, to));
    }
};

/* template <class A>
ctype_to(A&&) -> ctype_to<std::remove_cvref_t<A>>; */

struct ctype_is_any_tag
{
};

struct ctype_is_all_tag
{
};

template <typename T>
static constexpr auto _bind_back_ex(auto fn, T& arg)
{
    if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(void*) * 2)
        return bind_back(fn, arg);
    else
        return bind_back(fn, std::ref(arg));
}

template <class ArT, class AcT>
class ctype_is
{
    [[no_unique_address]] ArT adaptorRt_;
    [[no_unique_address]] AcT adaptorCt_;

  public:
    constexpr ctype_is() = default;

    using default_mode_t = ctype_is_all_tag;

    template <typename T>
    constexpr bool operator()(const T& item) const
    {
        if constexpr (std::is_class_v<T>)
        {
            // run default mode (rng)
            return this->operator()(item, default_mode_t());
        }
        else
        {
            if constexpr (std::invocable<AcT, T>)
            {
                if (std::is_constant_evaluated())
                    return adaptorCt_(item);
            }
            return adaptorRt_(item);
        }
    }

    template <class T>
    constexpr bool operator()(const T& rng, const ctype_is_any_tag) const
    {
        return std::ranges::any_of(rng, [this](auto... args) {
            this->operator()(args...);
        });
    }

    template <class T>
    constexpr bool operator()(const T& rng, const ctype_is_all_tag) const
    {
        return std::ranges::all_of(rng, [this](auto... args) {
            this->operator()(args...);
        });
    }

    template <typename T>
    constexpr bool operator()(const T* ptr, const ctype_is_any_tag) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (this->operator()(chr))
                return true;
        }
        return false;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr, const ctype_is_all_tag) const
    {
        for (auto chr = *ptr; chr != static_cast<T>('\0'); chr = *++ptr)
        {
            if (!this->operator()(chr))
                return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator()(const T* ptr)
    {
        return this->operator()(ptr, default_mode_t());
    }

    template <typename T, class Mode = default_mode_t>
    constexpr bool operator()(const T from, const size_t size, const Mode mode = {}) const
    {
        return this->operator()(std::span(from, size), mode);
    }

    template <typename T, class Mode = default_mode_t>
    constexpr bool operator()(const T from, const T to, const Mode mode = {}) const
    {
        return this->operator()(std::span(from, to), mode);
    }
};

/* template <class A>
ctype_is(A&&) -> ctype_is<std::remove_cvref_t<A>>; */

template <char From, char To>
static consteval auto _make_chars_range()
{
    static_assert(From < To);
    std::array<char, To - From + 1> buff;
    for (size_t i = 0; i < buff.size(); ++i)
        buff[i] = static_cast<char>(From + i);
    return buff;
}

template <size_t... S>
static constexpr auto _joint_arrays(const std::array<char, S>&... src)
{
    std::array<char, (S + ...)> buff;
    auto                        itr = buff.begin();

    ((itr = std::ranges::copy(src, itr).out), ...);
    return buff;
}

static constexpr auto _UpperChars  = _make_chars_range<'A', 'Z'>();
static constexpr auto _LowerChars  = _make_chars_range<'a', 'z'>();
static constexpr auto _NumberChars = _make_chars_range<'0', '9'>();
static constexpr auto _HexChars    = _joint_arrays(_NumberChars, _make_chars_range<'a', 'f'>(), _make_chars_range<'A', 'F'>());

static constexpr size_t _get_offset(const auto val, const auto& buff)
{
    const auto bg  = buff.begin();
    const auto end = buff.end();
    const auto pos = std::find(bg, end, val);
    return pos == end ? -1 : std::distance(bg, pos);
}

static constexpr auto _GetOffsetAt = []<typename T>(const T val, const auto& buff, const auto& buff2) -> T {
    const auto offset = _get_offset(val, buff);
    return offset == -1 ? val : buff2[offset];
};

template <typename Ct, typename Rt>
class ctype_buff
{
    Ct ct_;
    Rt rt_;

  public:
    constexpr ctype_buff(Ct ct, Rt rt)
        : ct_(ct)
        , rt_(rt)
    {
    }

  protected:
    template <typename Ret, typename T>
    constexpr Ret call(const T val) const
    {
        if constexpr (std::invocable<Ct, T>)
        {
            if (std::is_constant_evaluated())
                return ct_(val);
        }

        return rt_(val);
    }
};

template <typename... F>
struct ctype_to_impl : ctype_buff<F...>
{
    using ctype_buff<F...>::ctype_buff;

    constexpr char operator()(const char val) const
    {
        return this->template call<char>(val);
    }

    constexpr wchar_t operator()(const wchar_t val) const
    {
        return this->template call<wchar_t>(val);
    }
};

template <typename... F>
ctype_to_impl(F...) -> ctype_to_impl<std::remove_cvref_t<F>...>;

template <typename... F>
struct ctype_is_impl : ctype_buff<F...>
{
    using ctype_buff<F...>::ctype_buff;

    constexpr bool operator()(const char val) const
    {
        return this->template call<bool>(val);
    }

    constexpr bool operator()(const wchar_t val) const
    {
        return this->template call<bool>(val);
    }
};

template <typename... F>
ctype_is_impl(F...) -> ctype_is_impl<std::remove_cvref_t<F>...>;

static constexpr auto _overload_char(auto chr, auto wchr)
{
    return overload(
        [=](const char val) {
            return chr(val);
        },
        [=](const wchar_t val) {
            return wchr(val);
        }
    );
}

#ifdef __cpp_lib_ranges_contains
template <typename T>
static constexpr auto _copy_or_ref(T& val)
{
    if constexpr (std::copyable<T>)
        return val;
    else
        return std::ref(val);
}

static constexpr auto _Contains = _copy_or_ref(std::ranges::contains);
#else
static constexpr auto _Contains = [](auto& rng, auto val) {
    for (auto v : rng)
    {
        if (v == val)
            return true;
    }
    return false;
};
#endif

// ReSharper disable CppInconsistentNaming
constexpr ctype_to_impl to_lower(bind_back(_GetOffsetAt, _UpperChars, _LowerChars), overload(tolower, towlower));
constexpr ctype_to_impl to_upper(bind_back(_GetOffsetAt, _LowerChars, _UpperChars), overload(toupper, towupper));
constexpr ctype_is_impl is_alnum(nullptr, _overload_char(isalnum, iswalnum));
constexpr ctype_is_impl is_lower(bind_front(_Contains, _LowerChars), _overload_char(islower, iswlower));
constexpr ctype_is_impl is_upper(bind_front(_Contains, _UpperChars), _overload_char(isupper, iswupper));
constexpr ctype_is_impl is_digit(nullptr, _overload_char(isdigit, iswdigit));
constexpr ctype_is_impl is_xdigit(bind_front(_Contains, _HexChars), _overload_char(isxdigit, iswxdigit));
constexpr ctype_is_impl is_cntrl(nullptr, _overload_char(iscntrl, iswcntrl));
constexpr ctype_is_impl is_graph(nullptr, _overload_char(isgraph, iswgraph));
constexpr ctype_is_impl is_space(nullptr, _overload_char(isspace, iswspace));
constexpr ctype_is_impl is_print(nullptr, _overload_char(isprint, iswprint));
constexpr ctype_is_impl is_punct(nullptr, _overload_char(ispunct, iswpunct));
// ReSharper restore CppInconsistentNaming

} // namespace fd