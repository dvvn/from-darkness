module;

#include <concepts>
#include <string>
#include <tuple>
#include <utility>

export module fd.string;

// proxy class to force templates prebuild
template <typename C>
struct basic_string;
template <typename C>
struct basic_string_view;

template <typename C>
basic_string(const C*) -> basic_string<C>;
template <typename C>
basic_string(const basic_string_view<C>) -> basic_string<C>;

template <typename C>
basic_string_view(const C*) -> basic_string_view<C>;
template <typename C>
basic_string_view(const basic_string<C>&) -> basic_string_view<C>;

struct check_null_chr_t
{
};

constexpr check_null_chr_t check_null_chr;

template <class Tr, typename C>
constexpr bool _Ptr_equal(const C* ptr1, const C* ptr2, const size_t count)
{
    constexpr Tr traits;
    return traits.compare(ptr1, ptr2, count) == 0;
}

template <class Tr, typename C>
constexpr bool _Ptr_equal(const C* my_ptr, const C* unk_ptr, const size_t count, const check_null_chr_t)
{
    return _Ptr_equal<Tr>(my_ptr, unk_ptr, count) && unk_ptr[count] == static_cast<C>('\0');
}

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const basic_string_view<C> right)
{
    const auto lsize = left.size();
    return lsize == right.size() && _Ptr_equal<std::char_traits<C>>(left.data(), right.data(), lsize);
}

template <typename C>
constexpr bool operator!=(const basic_string_view<C> left, const basic_string_view<C> right)
{
    return !(left == right);
}

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const C* right)
{
    return _Ptr_equal<std::char_traits<C>>(left.data(), right, left.size(), check_null_chr);
}

template <typename C>
constexpr bool operator!=(const basic_string_view<C> left, const C* right)
{
    return !(left == right);
}

template <typename C>
constexpr bool operator==(const C* left, const basic_string_view<C> right)
{
    return (right == left);
}

template <typename C>
constexpr bool operator!=(const C* left, const basic_string_view<C> right)
{
    return !(left == right);
}

//----

#define _CONTAINS_FN                                                                      \
    template <class Tr>                                                                   \
    constexpr bool contains(const std::basic_string_view<value_type> strv) const noexcept \
    {                                                                                     \
        return _Base::find(strv) != _Base::npos;                                          \
    }                                                                                     \
    constexpr bool contains(const value_type chr) const noexcept                          \
    {                                                                                     \
        return _Base::find(chr) != _Base::npos;                                           \
    }                                                                                     \
    constexpr bool contains(const value_type* cstr) const                                 \
    {                                                                                     \
        return _Base::find(cstr) != _Base::npos;                                          \
    }

template <typename It, typename T>
It _Iter_or_this(It it, T*)
{
    return it;
}

template <typename S, typename T>
T& _Iter_or_this(S&, T* thisptr)
{
    return *thisptr;
}

#define _WRAP(_FN_)             \
    template <typename... Args> \
    constexpr decltype(auto) _FN_(Args&&... args)

#define _WRAP_ITER(_FN_)                                                      \
    _WRAP(_FN_)                                                               \
    {                                                                         \
        return _Iter_or_this(_Base::_FN_(std::forward<Args>(args)...), this); \
    }
#define _WRAP_THIS(_FN_)                          \
    _WRAP(_FN_)                                   \
    {                                             \
        _Base::_FN_(std::forward<Args>(args)...); \
        return *this;                             \
    }

#define _WRAP_VIEW      \
    _WRAP_THIS(substr); \
    /**/

#define _WRAP_STR           \
    _WRAP_ITER(insert);     \
    _WRAP_ITER(erase);      \
    _WRAP_THIS(append);     \
    _WRAP_THIS(operator+=); \
    _WRAP_THIS(replace);    \
    _WRAP_THIS(substr);

#if defined(__cpp_lib_string_contains) || 1
#define WRAP_basic_string_view
#define WRAP_basic_string
#else
#define WRAP_basic_string_view \
    _CONTAINS_FN               \
    _WRAP_VIEW
#define WRAP_basic_string \
    _CONTAINS_FN          \
    _WRAP_STR
#endif

//----

#define _PROVIDE(_C_, _NAME_)                                                                        \
    template <>                                                                                      \
    struct _NAME_<_C_> : std::_NAME_<_C_>                                                            \
    {                                                                                                \
        using _Base = std::_NAME_<_C_>;                                                              \
        template <typename... Args>                                                                  \
        constexpr _NAME_(Args&&... args) requires(std::constructible_from<_Base, decltype(args)...>) \
            : _Base(std::forward<Args>(args)...)                                                     \
        {                                                                                            \
        }                                                                                            \
        using _Base::value_type;                                                                     \
        using _Base::size_type;                                                                      \
        WRAP_##_NAME_;                                                                               \
    };

#define _ALL_TYPES(_FN_, ...)    \
    _FN_(char, __VA_ARGS__);     \
    _FN_(wchar_t, __VA_ARGS__);  \
    _FN_(char8_t, __VA_ARGS__);  \
    _FN_(char16_t, __VA_ARGS__); \
    _FN_(char32_t, __VA_ARGS__);

_ALL_TYPES(_PROVIDE, basic_string);
_ALL_TYPES(_PROVIDE, basic_string_view);

#define PREFIX_char
#define PREFIX_wchar_t  w
#define PREFIX_char8_t  u8
#define PREFIX_char16_t u16
#define PREFIX_char32_t u32

#define _ADD_PREFIX0(_PREFIX_, _NAME_) _PREFIX_##_NAME_
#define _ADD_PREFIX1(_PREFIX_, _NAME_) _ADD_PREFIX0(_PREFIX_, _NAME_)
#define _ADD_PREFIX(_C_, _NAME_)       _ADD_PREFIX1(PREFIX_##_C_, _NAME_)

#define _USE(_C_, _NAME_) using _ADD_PREFIX(_C_, _NAME_) = basic_##_NAME_<_C_>;

class string_builder
{
    template <typename T>
    static constexpr auto get_array_ptr(const T* ptr)
    {
        return ptr;
    }

    template <typename T>
    static constexpr size_t get_length(const T* ptr)
    {
        return std::char_traits<T>::length(ptr);
    }

    template <typename T>
    static constexpr auto extract_size(const T& obj)
    {
        if constexpr (std::is_class_v<T>)
            return std::pair(obj.data(), obj.size());
        else if constexpr (std::is_pointer_v<T>)
            return std::pair(obj, get_length(obj));
        else if constexpr (std::is_bounded_array_v<T>)
            return std::pair(get_array_ptr(obj), std::size(obj) - 1);
        else
            return std::pair(obj, 1);
    };

    template <class Buff, typename T, typename S>
    static constexpr void append_to(Buff& buff, const std::pair<T, S> obj)
    {
        auto [src, size] = obj;
        if constexpr (std::is_pointer_v<T>)
        {
            buff.append(src, src + size);
        }
        else
        {
            using val_t = Buff::value_type;
            do
                buff += static_cast<val_t>(src);
            while (--size > 0);
        }
    };

    template <typename T>
    static constexpr auto get_char_type(const T&)
    {
        if constexpr (std::is_class_v<T> || std::is_pointer_v<T> || std::is_bounded_array_v<T>)
            return std::iter_value_t<T>();
        else
            return T();
    }

    template <typename C, typename... Args>
    static constexpr auto get(const Args&... args)
    {
        basic_string<C> buff;
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
    template <class _Elem, class _UTy>
    static constexpr _Elem* _UIntegral_to_buff(_Elem* _RNext, _UTy _UVal)
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
                    *--_RNext = static_cast<_Elem>('0' + _UVal_chunk % 10);
                    _UVal_chunk /= 10;
                }
            }
        }

        auto _UVal_trunc = static_cast<unsigned long>(_UVal);
#endif // _WIN64

        do
        {
            *--_RNext = static_cast<_Elem>('0' + _UVal_trunc % 10);
            _UVal_trunc /= 10;
        }
        while (_UVal_trunc != 0);
        return _RNext;
    }

    template <class _Elem = char, typename _Ty>
    static constexpr basic_string<_Elem> _Integral_to_string(const _Ty _Val)
    {
        // convert _Val to string
        static_assert(is_integral_v<_Ty>, "_Ty must be integral");
        using _UTy = std::make_unsigned_t<_Ty>;
        _Elem _Buff[21]; // can hold -2^63 and 2^64 - 1, plus NUL
        _Elem* const _Buff_end = _STD end(_Buff);
        _Elem* _RNext          = _Buff_end;
        const auto _UVal       = static_cast<_UTy>(_Val);
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
    template <class _Elem = char, class _Ty>
    static constexpr basic_string<_Elem> _UIntegral_to_string(const _Ty _Val)
    {
        // convert _Val to string
        static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
        static_assert(std::is_unsigned_v<_Ty>, "_Ty must be unsigned");
        _Elem _Buff[21]; // can hold 2^64 - 1, plus NUL
        _Elem* const _Buff_end = std::end(_Buff);
        _Elem* const _RNext    = _UIntegral_to_buff(_Buff_end, _Val);
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

export namespace fd
{
    _ALL_TYPES(_USE, string);
    _ALL_TYPES(_USE, string_view);

    using ::basic_string;
    using ::basic_string_view;

    using ::operator==;
    using ::operator!=;

    constexpr string_builder make_string;
    constexpr to_string_impl to_string;
} // namespace fd
