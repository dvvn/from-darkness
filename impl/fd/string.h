// ReSharper disable CppUserDefinedLiteralSuffixDoesNotStartWithUnderscore
#pragma once

#include <algorithm>
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

    template <typename C>
    class to_string_impl
    {
        using string_t = basic_string<C>;

        // ReSharper disable All

        template <class _UTy>
        static constexpr C* _UIntegral_to_buff(C* _RNext, _UTy _UVal)
        {
            // format _UVal into buffer *ending at* _RNext
            static_assert(std::is_unsigned_v<_UTy>, "_UTy must be unsigned");

#ifdef _WIN64
            auto _UVal_trunc = _UVal;
#else
            constexpr bool _Big_uty = sizeof(_UTy) > 4;
            if constexpr (_Big_uty)
            {
                // For 64-bit numbers, work in chunks to avoid 64-bit divisions.
                while (_UVal > 0xFFFFFFFFU)
                {
                    auto _UVal_chunk = static_cast<unsigned long>(_UVal % 1000000000);
                    _UVal /= 1000000000;

                    for (uint8_t _Idx = 0; _Idx != 9; ++_Idx)
                    {
                        *--_RNext = static_cast<C>('0' + _UVal_chunk % 10);
                        _UVal_chunk /= 10;
                    }
                }
            }

            auto _UVal_trunc = static_cast<unsigned long>(_UVal);
#endif // _WIN64

            do
            {
                *--_RNext = static_cast<C>('0' + _UVal_trunc % 10);
                _UVal_trunc /= 10;
            }
            while (_UVal_trunc != 0);
            return _RNext;
        }

        template <typename _Ty>
        static constexpr string_t _Integral_to_string(const _Ty _Val)
        {
            // convert _Val to string
            static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
            using _UTy = std::make_unsigned_t<_Ty>;
            C _Buff[21]; // can hold -2^63 and 2^64 - 1, plus NUL
            const auto _Buff_end = std::end(_Buff);
            auto _RNext          = _Buff_end;
            const auto _UVal     = static_cast<_UTy>(_Val);
            if (_Val < 0)
            {
                _RNext    = _UIntegral_to_buff(_RNext, 0 - _UVal);
                *--_RNext = '-';
            }
            else
            {
                _RNext = _UIntegral_to_buff(_RNext, _UVal);
            }

            return { _RNext, _Buff_end };
        }

        // TRANSITION, CUDA - warning: pointless comparison of unsigned integer with zero
        template <class _Ty>
        static constexpr string_t _UIntegral_to_string(const _Ty _Val)
        {
            // convert _Val to string
            static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
            static_assert(std::is_unsigned_v<_Ty>, "_Ty must be unsigned");
            C _Buff[21]; // can hold 2^64 - 1, plus NUL
            const auto _Buff_end = std::end(_Buff);
            const auto _RNext    = _UIntegral_to_buff(_Buff_end, _Val);
            return { _RNext, _Buff_end };
        }

        // ReSharper restore All

      public:
        template <std::integral T>
        constexpr string_t operator()(const T val) const
        {
            if constexpr (std::is_unsigned_v<T>)
                return _UIntegral_to_string(val);
            else
                return _Integral_to_string(val);
        }

        constexpr string_t operator()(float val) const       = delete;
        constexpr string_t operator()(double val) const      = delete;
        constexpr string_t operator()(long double val) const = delete;
    };

    // ReSharper disable once CppInconsistentNaming
    constexpr to_string_impl<char> to_string;
    // ReSharper disable once CppInconsistentNaming
    constexpr to_string_impl<wchar_t> to_wstring;

    inline namespace literals
    {
        inline namespace string_literals
        {
            constexpr string operator"" s(const unsigned long long num)
            {
                return to_string(num);
            }

            /* constexpr string operator"" s(long double num)
            {
                return to_string(num);
            } */
        } // namespace string_literals
    }     // namespace literals

#pragma warning(pop)
} // namespace fd