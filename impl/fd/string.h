// ReSharper disable CppUserDefinedLiteralSuffixDoesNotStartWithUnderscore
#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <string>

namespace fd
{
#pragma warning(push)
#pragma warning(disable : 4455)

    using std::basic_string;
    using std::basic_string_view;
    using std::string;
    using std::string_view;
    using std::wstring;
    using std::wstring_view;
#ifdef __cpp_lib_char8_t
    using std::u8string;
    using std::u8string_view;
#endif
    using std::u16string;
    using std::u16string_view;
    using std::u32string;
    using std::u32string_view;

    static constexpr struct
    {
    } _CheckNullChr;

    template <typename C>
    static constexpr bool _ptr_equal(const C* ptr1, const C* ptr2, const size_t count)
    {
        return std::char_traits<C>::compare(ptr1, ptr2, count) == 0;
    }

    template <typename C>
    static constexpr bool _ptr_equal(const C* myPtr, const C* unkPtr, const size_t count, decltype(_CheckNullChr))
    {
        return _ptr_equal(myPtr, unkPtr, count) && unkPtr[count] == static_cast<C>('\0');
    }

    template <class R, class L>
    static constexpr bool _str_equal(const R& left, const L& right)
    {
        return left.size() == right.size() && _ptr_equal(left.data(), right.data(), left.size());
    }

    template <typename C>
    constexpr bool operator==(const basic_string_view<C> left, const basic_string_view<C> right)
    {
        // return _str_equal(left, right);
        const auto size = left.size();
        if (size != right.size())
            return false;

        const auto ld = left.data();
        const auto rd = right.data();

        if (ld == rd)
            return true;

        return _ptr_equal(ld, rd, size);
    }

    template <typename C>
    constexpr bool operator==(const basic_string_view<C> left, const basic_string<C>& right)
    {
        return _str_equal(left, right);
    }

    template <typename C>
    constexpr bool operator==(const basic_string<C>& left, const basic_string_view<C> right)
    {
        return _str_equal(left, right);
    }

    template <typename C>
    constexpr bool operator==(const basic_string_view<C> left, const C* right)
    {
        return _ptr_equal(left.data(), right, left.size(), _CheckNullChr);
    }

    template <typename C>
    constexpr bool operator==(const C* left, const basic_string_view<C> right)
    {
        return right == left;
    }

    //---

    template <typename C>
    constexpr bool operator==(const basic_string<C>& left, const basic_string<C>& right)
    {
        return left.size() == right.size() && _ptr_equal(left.data(), right.data(), left.size());
    }

    template <typename C>
    constexpr bool operator==(const basic_string<C>& left, const C* right)
    {
        return _ptr_equal(left.data(), right, left.size(), _CheckNullChr);
    }

    template <typename C>
    constexpr bool operator==(const C* left, const basic_string<C>& right)
    {
        return right == left;
    }

    template <typename T>
    constexpr size_t str_len(const T* str)
    {
        return std::char_traits<T>::length(str);
    }

    inline namespace literals
    {
        inline namespace string_view_literals
        {
            using std::string_view_literals::operator""sv;
        }

        inline namespace string_literals
        {
            using std::string_literals::operator""s;
        }
    } // namespace literals

    //--------------

    template <typename T>
    concept can_iter_value = requires { typename std::iter_value_t<T>; };

    template <typename T, bool = can_iter_value<T>>
    struct extract_value;

    template <typename T>
    struct extract_value<T, false>
    {
        using type = T;
    };

    template <typename T>
    struct extract_value<T, true>
    {
        using type = std::iter_value_t<T>;
    };

    template <typename T>
    concept can_reserve = requires(T obj) { obj.reserve(1234); };

    template <typename T, typename... Args>
    concept can_append = requires(T val, Args... args) { val.append(std::forward<Args>(args)...); };

    class write_string_impl
    {
        template <typename T>
        static constexpr auto extract_size(const T& obj)
        {
            if constexpr (std::is_class_v<T>)
                return std::pair(obj.begin(), obj.size());
            else if constexpr (std::is_pointer_v<T>)
                return std::pair(obj, str_len(obj));
            else if constexpr (std::is_bounded_array_v<T>)
                return std::pair(&obj[0], std::size(obj) - 1);
            else
                return std::pair(obj, static_cast<uint8_t>(1));
        }

        template <class Itr, typename T, typename S>
        static constexpr void append_to(Itr& buff, const std::pair<T, S>& obj)
        {
            auto [src, size] = obj;
            if (size == 0)
                return;

            using itr_t            = std::remove_cvref_t<Itr>;
            constexpr auto canCopy = std::is_pointer_v<T> || std::is_class_v<T> /* std::input_iterator<itr_t> */;
            if constexpr (std::input_or_output_iterator<itr_t>)
            {
                if constexpr (canCopy)
                    std::copy_n(src, size, buff);
                else
                    std::fill_n(buff, size, src);
            }
            else
            {
                if constexpr (!canCopy)
                    std::fill_n(std::back_insert_iterator(buff), size, src);
                else if constexpr (can_append<Itr&, T, T>)
                    buff.append(src, src + size);
                else
                    buff.insert(buff.end(), src, src + size);
            }
        }

      public:
        template <typename T, typename... Args>
        constexpr void operator()(T&& buff, const Args&... args) const
        {
            if constexpr (can_reserve<T&&>)
            {
                // ReSharper disable once CppInconsistentNaming
                const auto append_to_ex = [&](const auto&... p) {
                    const auto length = (static_cast<size_t>(p.second) + ...);
                    buff.reserve(length);
                    (append_to(buff, p), ...);
                };

                append_to_ex(extract_size(args)...);
            }
            else
            {
                // ReSharper disable once CppInconsistentNaming
                const auto append_to_ex = [&](const auto& p) {
                    append_to(buff, p);
                    buff += p.second;
                };

                (append_to_ex(extract_size(args)), ...);
            }
        }
    };

    struct make_string_impl
    {
        template <typename T, typename... Args>
        constexpr auto operator()(const T& arg1, const Args&... args) const
        {
            using char_type = typename extract_value<T>::type;
            basic_string<char_type> buff;
            constexpr write_string_impl write;
            write(buff, arg1, args...);
            return buff;
        }
    };

    // ReSharper disable once CppInconsistentNaming
    constexpr write_string_impl write_string;
    // ReSharper disable once CppInconsistentNaming
    constexpr make_string_impl make_string;

    // https://github.com/tcsullivan/constexpr-to-string

    template <typename C, typename T>
    static constexpr basic_string<C> _to_string(const T num, const uint8_t base)
    {
        constexpr auto digits = std::to_array("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
#ifdef _DEBUG
        if (base <= 1 || base >= digits.size())
        {
            std::invoke(std::get_terminate());
            return {};
        }
#endif
        if (num == 0)
            return {};

        basic_string<C> buff;

        size_t len = (num > 0 ? 0 : 1);
        for (auto n = num; n; ++len, n /= base)
        {
        }
        buff.resize(len);

        auto ptr = buff.data() + len;
        for (auto n = num; n; n /= base)
            *--ptr = digits[(num < 0 ? -1 : 1) * (n % base)];
        if (num < 0)
            *--ptr = '-';

        return buff;
    }

    template <typename C>
    static constexpr basic_string<C> _to_string(long double value, const bool trim, const uint8_t prec)
    {
#pragma warning(disable : 4244)
        int64_t whole = value;
        value -= whole;
        for (size_t i = 0; i < prec; i++)
            value *= 10;
        int64_t frac = value;

        //---------

        size_t len = 1;
        if (whole <= 0)
            len++;
        for (auto n = whole; n; len++, n /= 10)
        {
        }
        if (frac == 0 || (whole == 0 && frac < 0))
            len++;
        for (auto n = frac; n; len++, n /= 10)
        {
        }

        basic_string<C> buff;
        buff.resize(len);
        auto ptr = buff.data() + len;

        const auto append = [&ptr](const auto num) {
            if (num == 0)
            {
                *--ptr = '0';
            }
            else
            {
                for (auto n = num; n != 0; n /= 10)
                    *--ptr = (num < 0 ? -1 : 1) * (n % 10) + '0';
            }
        };

        append(frac);
        *--ptr = '.';
        append(whole);
        if (frac < 0 || whole < 0)
            *--ptr = '-';

        if (trim)
        {
            size_t offset = 0;
            for (auto itr = buff.rbegin(); itr != buff.rend(); ++itr)
            {
                if (*itr != '0')
                {
                    if (offset > 0)
                        buff.resize(buff.size() - offset);
                    break;
                }
                ++offset;
            }
        }
        return buff;
    }

    constexpr auto to_string(const std::integral auto num, const uint8_t base = 10)
    {
        return _to_string<char>(num, base);
    }

    constexpr auto to_wstring(const std::integral auto num, const uint8_t base = 10)
    {
        return _to_string<wchar_t>(num, base);
    }

    constexpr auto to_string(const std::floating_point auto num, const bool trim = false, const uint8_t prec = 5)
    {
        return _to_string<char>(num, trim, prec);
    }

    template <typename T>
    constexpr auto to_wstring(const std::floating_point auto num, const bool trim = false, const uint8_t prec = 5)
    {
        return _to_string<wchar_t>(num, trim, prec);
    }

    static_assert(to_string(1234u) == "1234");
    static_assert(to_string(-1234) == "-1234");
    static_assert(to_string(1.234, false, 5) == "1.23400");
    static_assert(to_string(1.234, true) == "1.234");
    static_assert(to_string(-1.23456) == "-1.23456");

    inline namespace literals
    {
        inline namespace string_literals
        {
            constexpr string operator"" s(const unsigned long long num)
            {
                return to_string(num);
            }

            constexpr string operator"" s(const long double num)
            {
                return to_string(num);
            }
        } // namespace string_literals
    }     // namespace literals

#pragma warning(pop)
} // namespace fd