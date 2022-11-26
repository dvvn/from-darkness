#pragma once

#define _CTYPE_DISABLE_MACROS

#include <fd/functional.h>

#include <cctype>
#include <cwctype>

#include <algorithm>
#include <array>
#include <limits>
#include <span>
#include <vector>

namespace fd
{
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
                std::vector<std::iter_value_t<T>> buff;
                buff.reserve(item.size());
                for (auto c : item)
                    buff.push_back(invoke(*this, c));
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

    constexpr size_t _Get_offset(const auto val, const auto& buff)
    {
        const auto bg  = buff.begin();
        const auto end = buff.end();
        const auto pos = std::find(bg, end, val);
        return pos == end ? -1 : std::distance(bg, pos);
    }

    constexpr auto _Get_offset_at = []<typename T>(const T val, const auto& buff, const auto& buff2) -> T {
        const auto offset = _Get_offset(val, buff);
        return offset == -1 ? val : buff2[offset];
    };

    template <typename Ct, typename Rt>
    class _Ctype_buff
    {
        Ct ct_;
        Rt rt_;

      public:
        constexpr _Ctype_buff(Ct ct, Rt rt)
            : ct_(ct)
            , rt_(rt)
        {
        }

      protected:
        template <typename Ret, typename T>
        constexpr Ret call(const T val) const
        {
            if constexpr (invocable<Ct, T>)
            {
                if (std::is_constant_evaluated())
                    return invoke(ct_, val);
            }

            return invoke(rt_, val);
        }
    };

    template <typename... F>
    struct _To_impl : _Ctype_buff<F...>
    {
        using _Ctype_buff<F...>::_Ctype_buff;

        constexpr char operator()(const char val) const
        {
            return this->call<char>(val);
        }

        constexpr wchar_t operator()(const wchar_t val) const
        {
            return this->call<wchar_t>(val);
        }
    };

    template <typename... F>
    _To_impl(const F...) -> _To_impl<F...>;

    template <typename... F>
    struct _Is_impl : _Ctype_buff<F...>
    {
        using _Ctype_buff<F...>::_Ctype_buff;

        constexpr bool operator()(const char val) const
        {
            return this->call<bool>(val);
        }

        constexpr bool operator()(const wchar_t val) const
        {
            return this->call<bool>(val);
        }
    };

    template <typename... F>
    _Is_impl(const F...) -> _Is_impl<F...>;

    constexpr _To_impl to_lower(bind_back(_Get_offset_at, upper_chars, lower_chars), overload(::tolower, ::towlower));
    constexpr _To_impl to_upper(bind_back(_Get_offset_at, lower_chars, upper_chars), overload(::toupper, ::towupper));

    /*constexpr auto _Contains = [](auto&&... args) {
        return std::ranges::contains(args...);
    };*/

    constexpr auto _Overload_char(auto chr, auto wchr)
    {
        return overload(
            [=](const char val) {
                return invoke(chr, val);
            },
            [=](const wchar_t val) {
                return invoke(wchr, val);
            });
    }

    template <typename T>
    constexpr auto _Copy_or_ref(T& val)
    {
        if constexpr (std::copyable<T>)
            return val;
        else
            return std::ref(val);
    }

    constexpr auto _Contains = _Copy_or_ref(std::ranges::contains);

    constexpr _Is_impl is_alnum(nullptr, _Overload_char(::isalnum, ::iswalnum));
    constexpr _Is_impl is_lower(bind_front(_Contains, lower_chars), _Overload_char(::islower, ::iswlower));
    constexpr _Is_impl is_upper(bind_front(_Contains, upper_chars), _Overload_char(::isupper, ::iswupper));
    constexpr _Is_impl is_digit(nullptr, _Overload_char(::isdigit, ::iswdigit));
    constexpr _Is_impl is_xdigit(bind_front(_Contains, hex_chars), _Overload_char(::isxdigit, ::iswxdigit));
    constexpr _Is_impl is_cntrl(nullptr, _Overload_char(::iscntrl, ::iswcntrl));
    constexpr _Is_impl is_graph(nullptr, _Overload_char(::isgraph, ::iswgraph));
    constexpr _Is_impl is_space(nullptr, _Overload_char(::isspace, ::iswspace));
    constexpr _Is_impl is_print(nullptr, _Overload_char(::isprint, ::iswprint));
    constexpr _Is_impl is_punct(nullptr, _Overload_char(::ispunct, ::iswpunct));
} // namespace fd
