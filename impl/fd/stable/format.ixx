module;

#if 0
#include <format>
#else
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#endif

#include <tuple>

export module fd.format;
export import fd.string;

#ifdef FMT_VERSION
#define _FMT fmt
#else
#define _FMT std
#endif

/* namespace _FMT
{
    template <typename C>
    struct formatter<fd::basic_string_view<C>, C> : formatter<basic_string_view<C>, C>
    {
        template <class FormatContext>
        auto format(const fd::basic_string_view<C> str, FormatContext& fc) const
        {
            const basic_string_view<C> tmp(str.data(), str.size());
            return formatter<basic_string_view<C>, C>::format(tmp, fc);
        }
    };

    template <typename C>
    struct formatter<fd::basic_string<C>, C> : formatter<fd::basic_string_view<C>, C>
    {
    };
} // namespace _FMT */

class string_builder
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
            return std::pair(obj, 1);
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

    template <class Buff, typename T, typename S>
    static constexpr void append_to(Buff& buff, const std::pair<T, S> obj)
    {
        auto [src, size] = obj;
        if constexpr (std::is_pointer_v<T> || std::is_class_v<T>)
        {
            buff.append(src, src + size);
        }
        else
        {
            using val_t = Buff::value_type;
            do
                buff.push_back(static_cast<val_t>(src));
            while (--size > 0);
        }
    };

    template <typename C, typename... Args>
    static constexpr auto get(const Args&... args)
    {
        fd::basic_string<C> buff;
        if constexpr (sizeof...(Args) == 1)
        {
            const auto pair = extract_size(args...);
            append_to(buff, pair);
        }
        else
        {
            std::apply(
                [&buff](auto... pairs) {
                    {
                        const auto length = (pairs.second + ...);
                        buff.reserve(length);
                    }
                    (append_to(buff, pairs), ...);
                },
                std::tuple(extract_size(args)...));
        }
        return buff;
    }

  public:
    template <typename A, typename... Args>
    constexpr auto operator()(const A& arg1, const Args&... args) const
    {
        static_assert(!std::is_empty_v<A>); // hack to test second overload
        using char_t = decltype(get_char_type(arg1));
        return get<char_t>(arg1, args...);
    }

    template <typename C, typename... Args>
    constexpr auto operator()(const std::in_place_type_t<C>, const Args&... args) const
    {
        return get<C>(args...);
    }
};

class to_string_impl
{
    template <class _UTy>
    static constexpr char* _UIntegral_to_buff(char* _RNext, _UTy _UVal)
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

                for (int _Idx = 0; _Idx != 9; ++_Idx)
                {
                    *--_RNext = static_cast<char>('0' + _UVal_chunk % 10);
                    _UVal_chunk /= 10;
                }
            }
        }

        auto _UVal_trunc = static_cast<unsigned long>(_UVal);
#endif // _WIN64

        do
        {
            *--_RNext = static_cast<char>('0' + _UVal_trunc % 10);
            _UVal_trunc /= 10;
        }
        while (_UVal_trunc != 0);
        return _RNext;
    }

    template <typename _Ty>
    static constexpr fd::string _Integral_to_string(const _Ty _Val)
    {
        // convert _Val to string
        static_assert(is_integral_v<_Ty>, "_Ty must be integral");
        using _UTy = std::make_unsigned_t<_Ty>;
        char _Buff[21]; // can hold -2^63 and 2^64 - 1, plus NUL
        char* const _Buff_end = _STD end(_Buff);
        char* _RNext          = _Buff_end;
        const auto _UVal      = static_cast<_UTy>(_Val);
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
    static constexpr fd::string _UIntegral_to_string(const _Ty _Val)
    {
        // convert _Val to string
        static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
        static_assert(std::is_unsigned_v<_Ty>, "_Ty must be unsigned");
        char _Buff[21]; // can hold 2^64 - 1, plus NUL
        char* const _Buff_end = std::end(_Buff);
        char* const _RNext    = _UIntegral_to_buff(_Buff_end, _Val);
        return { _RNext, _Buff_end };
    }

  public:
    template <std::integral T>
    constexpr auto operator()(const T val) const
    {
        if constexpr (std::is_unsigned_v<T>)
            return _UIntegral_to_string(val);
        else
            return _Integral_to_string(val);
    }

    template <std::floating_point T>
    constexpr void operator()(const T val) const = delete; // not implemented
};

class format_impl
{
    template <typename C, typename... Args>
    static auto make_format_args(const Args&... args)
    {
        if constexpr (std::same_as<C, char>)
            return _FMT::make_format_args(args...);
        else if constexpr (std::same_as<C, wchar_t>)
            return _FMT::make_wformat_args(args...);
    }

    template <typename C, typename... Args>
    static auto impl(const fd::basic_string_view<C> fmt, const Args&... args)
    {
        static_assert(sizeof...(Args) > 0);
        fd::basic_string<C> buff;
        _FMT::vformat_to(std::back_inserter(buff), fmt, make_format_args<C>(args...));
        return buff;
    }

  public:
    template <typename... Args>
    auto operator()(const fd::string_view fmt, const Args&... args) const
    {
        return impl(fmt, args...);
    }

    template <typename... Args>
    auto operator()(const fd::wstring_view fmt, const Args&... args) const
    {
        return impl(fmt, args...);
    }
};

export namespace fd
{
    constexpr string_builder make_string;
    constexpr to_string_impl to_string;
    constexpr format_impl format;
} // namespace fd
