module;

#include <algorithm>
#include <iterator>
#include <utility>

export module fd.string.make;
export import fd.string;

namespace fd
{
    /*  template <typename T>
     using get_char_t = decltype([] {
         if constexpr (std::is_pointer_v<T> || std::is_bounded_array_v<T>)
             return std::iter_value_t<T>();
         else if constexpr (std::is_class_v<T>)
             return decltype(*std::declval<T>().begin())();
         else
             return T();
     })(); */

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

            using itr_t             = std::remove_cvref_t<Itr>;
            constexpr auto can_copy = std::is_pointer_v<T> || std::is_class_v<T> /* std::input_iterator<itr_t> */;
            if constexpr (std::input_or_output_iterator<itr_t>)
            {
                if constexpr (can_copy)
                    std::copy_n(src, size, buff);
                else
                    std::fill_n(buff, size, src);
            }
            else
            {
                if constexpr (can_copy)
                    buff.append(src, src + size);
                else
                    std::fill_n(std::back_insert_iterator(buff), size, src);
            }
        }

      public:
        template <typename T, typename... Args>
        constexpr void operator()(T&& buff, const Args&... args) const
        {
            if constexpr (can_reserve<T&&>)
            {
                const auto append_to_ex = [&](const auto&... p) {
                    const auto length = (static_cast<size_t>(p.second) + ...);
                    buff.reserve(length);
                    (append_to(buff, p), ...);
                };

                append_to_ex(extract_size(args)...);
            }
            else
            {
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
            using char_type = extract_value<T>::type;
            basic_string<char_type> buff;
            constexpr write_string_impl write;
            write(buff, arg1, args...);
            return buff;
        }
    };

    export constexpr write_string_impl write_string;
    export constexpr make_string_impl make_string;

    template <typename C>
    class to_string_impl
    {
        using _String = basic_string<C>;

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
        static constexpr _String _Integral_to_string(const _Ty _Val)
        {
            // convert _Val to string
            static_assert(is_integral_v<_Ty>, "_Ty must be integral");
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
        static constexpr _String _UIntegral_to_string(const _Ty _Val)
        {
            // convert _Val to string
            static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
            static_assert(std::is_unsigned_v<_Ty>, "_Ty must be unsigned");
            C _Buff[21]; // can hold 2^64 - 1, plus NUL
            const auto _Buff_end = std::end(_Buff);
            const auto _RNext    = _UIntegral_to_buff(_Buff_end, _Val);
            return { _RNext, _Buff_end };
        }

      public:
        constexpr _String operator()(const int32_t val) const
        {
            return _Integral_to_string(val);
        }

        constexpr _String operator()(const uint32_t val) const
        {
            return _UIntegral_to_string(val);
        }

        constexpr _String operator()(const long val) const
        {
            return _Integral_to_string(val);
        }

        constexpr _String operator()(const unsigned long val) const
        {
            return _UIntegral_to_string(val);
        }

        constexpr _String operator()(const int64_t val) const
        {
            return _Integral_to_string(val);
        }

        constexpr _String operator()(const uint64_t val) const
        {
            return _UIntegral_to_string(val);
        }

        constexpr _String operator()(const float val) const       = delete;
        constexpr _String operator()(const double val) const      = delete;
        constexpr _String operator()(const long double val) const = delete;
    };

    export constexpr to_string_impl<char> to_string;
    export constexpr to_string_impl<wchar_t> to_wstring;

    export inline namespace literals
    {
        inline namespace string_literals
        {
            constexpr string operator"" s(unsigned long long num)
            {
                return to_string(num);
            }

            /* constexpr string operator"" s(long double num)
            {
                return to_string(num);
            } */
        } // namespace string_literals
    }
} // namespace fd
