module;

#include <algorithm>
#include <iterator>
#include <tuple>

export module fd.string.make;
export import fd.string;

class string_maker
{
    template <typename T>
    static constexpr auto get_array_ptr(const T* ptr)
    {
        return ptr;
    }

    template <typename T>
    static constexpr auto extract_size(const T& obj)
    {
        if constexpr (std::is_class_v<T>)
            return std::pair(obj.begin(), obj.size());
        else if constexpr (std::is_pointer_v<T>)
            return std::pair(obj, fd::str_len(obj));
        else if constexpr (std::is_bounded_array_v<T>)
            return std::pair(get_array_ptr(obj), std::size(obj) - 1);
        else
            return std::pair(obj, static_cast<uint8_t>(1));
    };

    template <class Itr, typename T, typename S>
    static constexpr void append_to(Itr&& buff, const std::pair<T, S>& obj)
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
                std::fill_n(std::back_inserter(buff), size, src);
        }
    };

    template <typename T>
    static constexpr auto get_char_type(const T& obj)
    {
        if constexpr (std::is_pointer_v<T> || std::is_bounded_array_v<T>)
            return std::iter_value_t<T>();
        else if constexpr (std::is_class_v<T>)
            return std::remove_cvref_t<decltype(*obj.begin())>();
        else
            return T();
    }

  public:
    template <typename Itr, typename... Args>
    constexpr void write(Itr buff, const Args&... args) const
    {
        const auto append_to_ex = [&](const auto& p) {
            append_to(buff, p);
            buff += p.second;
        };

        std::apply(
            [&](const auto&... pairs) {
                (append_to_ex(pairs), ...);
            },
            std::make_tuple(extract_size(args)...));
    }

    template <typename T, typename... Args>
    constexpr void write(T& buff, const Args&... args) const requires(!std::is_const_v<T>)
    {
        std::apply(
            [&buff](const auto&... pairs) {
                const auto length = (static_cast<size_t>(pairs.second) + ...);
                buff.reserve(length);
                (append_to(buff, pairs), ...);
            },
            std::make_tuple(extract_size(args)...));
    }

    template <typename A, typename... Args>
    constexpr auto operator()(const A& arg1, const Args&... args) const
    {
        using char_t = decltype(get_char_type(arg1));
        fd::basic_string<char_t> buff;
        write(buff, arg1, args...);
        return buff;
    }
};

template <typename C>
class to_string_impl
{
    using _String = fd::basic_string<C>;

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

export namespace fd
{
    constexpr string_maker make_string;
    constexpr to_string_impl<char> to_string;
    constexpr to_string_impl<wchar_t> to_wstring;

    inline namespace literals
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
    }     // namespace literals
} // namespace fd
