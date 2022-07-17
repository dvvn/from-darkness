module;

#include <fd/utility.h>

#include <string>
#include <tuple>
#include <utility>

export module fd.string;
export import fd.hash;

struct check_null_chr_t
{
};

constexpr check_null_chr_t check_null_chr;

template <typename C>
constexpr bool _Ptr_equal(const C* ptr1, const C* ptr2, const size_t count)
{
    return std::char_traits<C>::compare(ptr1, ptr2, count) == 0;
}

template <typename C>
constexpr bool _Ptr_equal(const C* my_ptr, const C* unk_ptr, const size_t count, const check_null_chr_t)
{
    return _Ptr_equal(my_ptr, unk_ptr, count) && unk_ptr[count] == static_cast<C>('\0');
}

//-----

template <typename T>
concept is_iterator = requires(T it)
{
    *it;
    ++it;
};

template <typename Fn, typename T>
constexpr decltype(auto) _Iter_this_void(Fn fn, T* thisptr)
{
    using ret_val = decltype(std::invoke(fn));
    if constexpr (std::is_void_v<ret_val>)
    {
        std::invoke(fn);
    }
    else
    {
        decltype(auto) tmp = std::invoke(fn);
        if constexpr (is_iterator<ret_val> || !std::is_class_v<std::remove_reference_t<ret_val>>)
            return tmp;
        else if constexpr (std::is_lvalue_reference_v<ret_val>)
            return *thisptr;
        else
            return std::remove_const_t<T>(std::forward<decltype(tmp)>(tmp));
    }
}

#define _PROXY_INNER
#define _PROXY_INNERconst

#define _PROXY_IMPL(_FN_, _CONST_)                             \
    template <typename... Args>                                \
    constexpr decltype(auto) _FN_(Args&&... args) _CONST_      \
    {                                                          \
        _PROXY_INNER##_CONST_;                                 \
        return _Iter_this_void(                                \
            [&]() -> decltype(auto) {                          \
                return str_._FN_(std::forward<Args>(args)...); \
            },                                                 \
            this);                                             \
    }

#define _PROXY(_FN_)       _PROXY_IMPL(_FN_, )
#define _PROXY_CONST(_FN_) _PROXY_IMPL(_FN_, const)
#define _PROXY_ALL(_FN_)   _PROXY(_FN_) _PROXY_CONST(_FN_)

#define _CONTAINS_SPECIAL                                                                \
    constexpr bool contains(const basic_string_view<value_type> strv) const              \
    {                                                                                    \
        _PROXY_INNERconst;                                                               \
        return str_.find(std::basic_string_view(strv.data(), strv.size())) != str_.npos; \
    }

#define _FIND_SPECIAL                                                                                    \
    constexpr size_type find(const basic_string_view<value_type> strv, const size_type offset = 0) const \
    {                                                                                                    \
        _PROXY_INNERconst;                                                                               \
        return str_.find(std::basic_string_view(strv.data(), strv.size()), offset);                      \
    }

#define _APPEND_SPECIAL                                              \
    constexpr auto& append(const basic_string_view<value_type> strv) \
    {                                                                \
        _PROXY_INNER;                                                \
        str_.append(strv.data(), strv.size());                       \
        return *this;                                                \
    }

#define _ASSIGN_SPECIAL                                              \
    constexpr auto& assign(basic_string&& str)                       \
    {                                                                \
        _PROXY_INNER;                                                \
        str_.assign(std::move(str.str_));                            \
        return *this;                                                \
    }                                                                \
    constexpr auto& assign(const basic_string_view<value_type> strv) \
    {                                                                \
        _PROXY_INNER;                                                \
        str_.assign(strv.data(), strv.size());                       \
        return *this;                                                \
    }

#define _PLUS_ASSIGN_SPECIAL                                             \
    constexpr auto& operator+=(const basic_string_view<value_type> strv) \
    {                                                                    \
        return this->append(strv);                                       \
    }

#ifdef __cpp_lib_string_contains
#define _CONTAINS_FN       \
    _PROXY_CONST(contains) \
    _CONTAINS_SPECIAL
#else
#define _CONTAINS_FN                                             \
    constexpr bool contains(const value_type chr) const noexcept \
    {                                                            \
        _PROXY_INNERconst;                                       \
        return str_.find(chr) != str_.npos;                      \
    }                                                            \
    constexpr bool contains(const value_type* cstr) const        \
    {                                                            \
        _PROXY_INNERconst;                                       \
        return str_.find(cstr) != str_.npos;                     \
    }                                                            \
    _CONTAINS_SPECIAL
#endif

#define _PROXY_VIEW                                                                                                                           \
    FOR_EACH(_PROXY_CONST, begin, end, operator[], at, front, back, data, size, max_size, empty, substr, starts_with, ends_with, find, rfind) \
    FOR_EACH(_PROXY, remove_prefix, remove_suffix)                                                                                            \
    _FIND_SPECIAL _CONTAINS_FN

#define _PROXY_STR                                                                                                                                                     \
    FOR_EACH(_PROXY_CONST, get_allocator, empty, size, max_size, capacity, find, rfind)                                                                                \
    FOR_EACH(_PROXY, operator=, assign, reserve, shrink_to_fit, clear, insert, erase, push_back, pop_back, append, operator+=, starts_with, ends_with, substr, resize) \
    FOR_EACH(_PROXY_ALL, at, operator[], front, back, data, c_str, begin, end)                                                                                         \
    _FIND_SPECIAL _CONTAINS_FN _APPEND_SPECIAL _ASSIGN_SPECIAL _PLUS_ASSIGN_SPECIAL

#define _ALL_TYPES(_FN_, ...)      \
    _FN_(char, ##__VA_ARGS__);     \
    _FN_(wchar_t, ##__VA_ARGS__);  \
    _FN_(char8_t, ##__VA_ARGS__);  \
    _FN_(char16_t, ##__VA_ARGS__); \
    _FN_(char32_t, ##__VA_ARGS__);

#define PREFIX_char
#define PREFIX_wchar_t  w
#define PREFIX_char8_t  u8
#define PREFIX_char16_t u16
#define PREFIX_char32_t u32

#define _ADD_PREFIX0(_PREFIX_, _NAME_) _PREFIX_##_NAME_
#define _ADD_PREFIX1(_PREFIX_, _NAME_) _ADD_PREFIX0(_PREFIX_, _NAME_)
#define _ADD_PREFIX(_C_, _NAME_)       _ADD_PREFIX1(PREFIX_##_C_, _NAME_)

#define _USING(_C_, _NAME_) using _ADD_PREFIX(_C_, _NAME_) = basic_##_NAME_<_C_>;

#define _PROVIDE_BASIC(_BASE_) \
    using _Base = _BASE_;      \
    _Base str_;

#define _PROVIDE_EX
#define _CONSTRUCT_INNER

#define _REUSE(_NAME_) using _NAME_ = typename _Base::_NAME_;

#define _FWD_USINGS                                                                                                                                                             \
    FOR_EACH(                                                                                                                                                                   \
        _REUSE, value_type, pointer, const_pointer, reference, const_reference, const_iterator, iterator, const_reverse_iterator, reverse_iterator, size_type, difference_type) \
    static constexpr size_type npos = _Base::npos;

#define _PROVIDE(_C_, _NAME_, _BASE_, _PROXY_)                                                                                                     \
    template <>                                                                                                                                    \
    class _NAME_<_C_>                                                                                                                              \
    {                                                                                                                                              \
        _PROVIDE_BASIC(_BASE_<_C_>);                                                                                                               \
        _PROVIDE_EX;                                                                                                                               \
                                                                                                                                                   \
      public:                                                                                                                                      \
        _FWD_USINGS;                                                                                                                               \
        constexpr _NAME_() /* requires(std::default_initializable<_Base>)  */                                                                      \
        {                                                                                                                                          \
            _CONSTRUCT_INNER;                                                                                                                      \
        }                                                                                                                                          \
        constexpr _NAME_(const _Base& str) /* requires(std::copyable<Base>)*/                                                                      \
            : str_(str)                                                                                                                            \
        {                                                                                                                                          \
            _CONSTRUCT_INNER;                                                                                                                      \
        }                                                                                                                                          \
        constexpr _NAME_(_Base&& str) /* requires(std::movable<Base>)*/                                                                            \
            : str_(std::move(str))                                                                                                                 \
        {                                                                                                                                          \
            _CONSTRUCT_INNER;                                                                                                                      \
        }                                                                                                                                          \
        template <class It, class End>                                                                                                             \
        constexpr _NAME_(It first, End last) requires(std::constructible_from<_Base, It, End>)                                                     \
            : str_(first, last)                                                                                                                    \
        {                                                                                                                                          \
            _CONSTRUCT_INNER;                                                                                                                      \
        }                                                                                                                                          \
        template <class T>                                                                                                                         \
        constexpr _NAME_(const T& other) requires(std::is_class_v<T> && !std::same_as<T, _Base> && std::same_as<std::iter_value_t<T>, value_type>) \
            : str_(other.data(), other.size())                                                                                                     \
        {                                                                                                                                          \
            _CONSTRUCT_INNER;                                                                                                                      \
        }                                                                                                                                          \
        template <class T>                                                                                                                         \
        constexpr _NAME_(const T* other) requires(std::same_as<T, value_type>)                                                                     \
            : str_(other)                                                                                                                          \
        {                                                                                                                                          \
            _CONSTRUCT_INNER;                                                                                                                      \
        }                                                                                                                                          \
        _PROXY_;                                                                                                                                   \
    };

//--------------------

#pragma warning(disable : 4005)

template <typename C>
struct basic_string_view;
template <typename C>
struct basic_string;

template <typename C>
basic_string(const C*) -> basic_string<C>;
template <typename C>
basic_string(const basic_string_view<C>) -> basic_string<C>;

template <typename C>
basic_string_view(const C*) -> basic_string_view<C>;
template <typename C>
basic_string_view(const basic_string<C>&) -> basic_string_view<C>;
template <typename C>
basic_string_view(basic_string<C>&&) -> basic_string_view<void>;

_ALL_TYPES(_PROVIDE, basic_string_view, std::basic_string_view, _PROXY_VIEW);
_ALL_TYPES(_PROVIDE, basic_string, std::basic_string, _PROXY_STR);

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const basic_string_view<C> right)
{
    const auto lsize = left.size();
    return lsize == right.size() && _Ptr_equal(left.data(), right.data(), lsize);
}

template <typename C>
constexpr bool operator==(const basic_string_view<C> left, const C* right)
{
    return _Ptr_equal(left.data(), right, left.size(), check_null_chr);
}

template <typename C>
constexpr bool operator==(const C* left, const basic_string_view<C> right)
{
    return (right == left);
}

//--------------------

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

//---

template <typename Chr, size_t Size>
struct trivial_chars_cache
{
    Chr arr[Size];

    constexpr trivial_chars_cache(const Chr* str_source)
    {
        std::copy_n(str_source, Size, arr);
    }
};

namespace fd
{
    template <typename Chr, size_t Size>
    struct hash<trivial_chars_cache<Chr, Size>>
    {
        constexpr size_t operator()(const trivial_chars_cache<Chr, Size>& str) const
        {
            return _Hash_bytes(str.arr, Size - 1);
        }
    };

    export template <typename C>
    struct hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string_view<C> str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };

    export template <typename C>
    struct hash<basic_string<C>> : hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string<C>& str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };
} // namespace fd

template <typename Chr, size_t Size>
trivial_chars_cache(const Chr (&arr)[Size]) -> trivial_chars_cache<Chr, Size>;

template <trivial_chars_cache Cache>
consteval size_t operator"" _hash()
{
    return fd::_Hash_bytes(Cache.arr, std::size(Cache.arr) - 1);
}

static_assert("test"_hash == u8"test"_hash);
static_assert(u"test"_hash == "t\0e\0s\0t\0"_hash);
static_assert(U"test"_hash == u"t\0e\0s\0t\0"_hash);
static_assert(U"ab"_hash == "a\0\0\0b\0\0\0"_hash);

export namespace fd
{
    _ALL_TYPES(_USING, string);
    _ALL_TYPES(_USING, string_view);

    using ::basic_string;
    using ::basic_string_view;

    using ::operator==;

    constexpr string_builder make_string;
    constexpr to_string_impl to_string;

    //----

    inline namespace literals
    {
        using ::operator"" _hash;
    }

} // namespace fd
