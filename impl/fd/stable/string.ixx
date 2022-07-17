module;

#include <fd/utility.h>

#include <string>
#include <tuple>
#include <utility>

export module fd.string;
export import fd.hash;
import fd.lazy_invoke;

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
constexpr decltype(auto) _Item_this_void(Fn fn, T* thisptr)
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
        return _Item_this_void(                                \
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

#define _ASSIGN_OP_SPECIAL                                              \
    constexpr auto& operator=(const basic_string_view<value_type> strv) \
    {                                                                   \
        return this->assign(strv);                                      \
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
    _FIND_SPECIAL _CONTAINS_FN _APPEND_SPECIAL _ASSIGN_SPECIAL _PLUS_ASSIGN_SPECIAL _ASSIGN_OP_SPECIAL

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

#define _EMPTY

#define _USING(_C_, _NAME_) using _ADD_PREFIX(_C_, _NAME_) = basic_##_NAME_<_C_>;

#define _REUSE(_NAME_) using _NAME_ = typename _Base::_NAME_;

#define _PROVIDE(_C_, _NAME_, _BASE_, _PRIVATE_, _CONSTRUCT_, _PUBLIC_)                                                                            \
    template <>                                                                                                                                    \
    class _NAME_<_C_>                                                                                                                              \
    {                                                                                                                                              \
        using _Base = _BASE_<_C_>;                                                                                                                 \
        _Base str_;                                                                                                                                \
        _PRIVATE_;                                                                                                                                 \
                                                                                                                                                   \
      public:                                                                                                                                      \
        FOR_EACH(_REUSE,                                                                                                                           \
                 value_type,                                                                                                                       \
                 pointer,                                                                                                                          \
                 const_pointer,                                                                                                                    \
                 reference,                                                                                                                        \
                 const_reference,                                                                                                                  \
                 const_iterator,                                                                                                                   \
                 iterator,                                                                                                                         \
                 const_reverse_iterator,                                                                                                           \
                 reverse_iterator,                                                                                                                 \
                 size_type,                                                                                                                        \
                 difference_type)                                                                                                                  \
        static constexpr size_type npos = _Base::npos;                                                                                             \
        constexpr _NAME_() /* requires(std::default_initializable<_Base>)  */                                                                      \
            : str_()                                                                                                                               \
        {                                                                                                                                          \
            _CONSTRUCT_;                                                                                                                           \
        }                                                                                                                                          \
        constexpr _NAME_(const _Base& str) /* requires(std::copyable<Base>)*/                                                                      \
            : str_(str)                                                                                                                            \
        {                                                                                                                                          \
            _CONSTRUCT_;                                                                                                                           \
        }                                                                                                                                          \
        constexpr _NAME_(_Base&& str) /* requires(std::movable<Base>)*/                                                                            \
            : str_(std::move(str))                                                                                                                 \
        {                                                                                                                                          \
            _CONSTRUCT_;                                                                                                                           \
        }                                                                                                                                          \
        template <class It, class End>                                                                                                             \
        constexpr _NAME_(It first, End last) requires(std::constructible_from<_Base, It, End>)                                                     \
            : str_(first, last)                                                                                                                    \
        {                                                                                                                                          \
            _CONSTRUCT_;                                                                                                                           \
        }                                                                                                                                          \
        template <class T>                                                                                                                         \
        constexpr _NAME_(const T& other) requires(std::is_class_v<T> && !std::same_as<T, _Base> && std::same_as<std::iter_value_t<T>, value_type>) \
            : str_(other.data(), other.size())                                                                                                     \
        {                                                                                                                                          \
            _CONSTRUCT_;                                                                                                                           \
        }                                                                                                                                          \
        template <class T>                                                                                                                         \
        constexpr _NAME_(const T* other) requires(std::same_as<T, value_type>)                                                                     \
            : str_(other)                                                                                                                          \
        {                                                                                                                                          \
            _CONSTRUCT_;                                                                                                                           \
        }                                                                                                                                          \
        _PUBLIC_;                                                                                                                                  \
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

_ALL_TYPES(_PROVIDE, basic_string_view, std::basic_string_view, _EMPTY, _EMPTY, _PROXY_VIEW);
_ALL_TYPES(_PROVIDE, basic_string, std::basic_string, _EMPTY, _EMPTY, _PROXY_STR);

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
    return right == left;
}

//--------------------

template <typename T>
struct basic_hashed_string_view;
template <typename T>
struct basic_hashed_string;

template <typename C>
basic_hashed_string(const C*) -> basic_hashed_string<C>;
template <typename C>
basic_hashed_string(const basic_string_view<C>) -> basic_hashed_string<C>;

template <typename C>
basic_hashed_string_view(const C*) -> basic_hashed_string_view<C>;
template <typename C>
basic_hashed_string_view(const basic_string<C>&) -> basic_hashed_string_view<C>;
template <typename C>
basic_hashed_string_view(basic_string<C>&&) -> basic_hashed_string_view<void>;
template <typename C>
basic_hashed_string_view(const basic_string_view<C>) -> basic_hashed_string_view<C>;

#define _FIND_SPECIAL
#define _CONTAINS_FN _PROXY_CONST(contains)
#define _APPEND_SPECIAL
#define _ASSIGN_SPECIAL
#define _PLUS_ASSIGN_SPECIAL
#define _ASSIGN_OP_SPECIAL

#define _PRIVATE_HASH                            \
    size_t hash_;                                \
    constexpr void _Calc_hash()                  \
    {                                            \
        hash_ = fd::_Hash_bytes(data(), size()); \
    }
#define _PROXY_INNER                          \
    const fd::lazy_invoke update_hash = [&] { \
        _Calc_hash();                         \
    };

#define _PUBLIC_HASH                 \
    constexpr size_type hash() const \
    {                                \
        return hash_;                \
    }

#define _PROXY_VIEW_HASH \
    _PROXY_VIEW;         \
    _PUBLIC_HASH;

#define _PROXY_STR_HASH \
    _PROXY_STR;         \
    _PUBLIC_HASH;

_ALL_TYPES(_PROVIDE, basic_hashed_string_view, basic_string_view, _PRIVATE_HASH, _Calc_hash(), _PROXY_VIEW_HASH);
_ALL_TYPES(_PROVIDE, basic_hashed_string, basic_string, _PRIVATE_HASH, _Calc_hash(), _PROXY_STR_HASH);

template <typename C>
constexpr bool operator==(const basic_hashed_string_view<C> left, const basic_hashed_string_view<C> right)
{
    return left.hash() == right.hash();
}

template <typename C>
constexpr bool operator==(const basic_hashed_string_view<C> left, const size_t right)
{
    return left.hash() == right;
}

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

template <typename Chr, size_t Size>
trivial_chars_cache(const Chr (&arr)[Size]) -> trivial_chars_cache<Chr, Size>;

export namespace fd
{
    _ALL_TYPES(_USING, string);
    _ALL_TYPES(_USING, string_view);

    using ::basic_string;
    using ::basic_string_view;

    _ALL_TYPES(_USING, hashed_string);
    _ALL_TYPES(_USING, hashed_string_view);

    using ::basic_hashed_string;
    using ::basic_hashed_string_view;

    using ::operator==;

    template <typename C>
    struct hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string_view<C> str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };

    template <typename C>
    struct hash<basic_string<C>> : hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string<C>& str) const
        {
            return _Hash_bytes(str.data(), str.size());
        }
    };

    inline namespace literals
    {
        template <trivial_chars_cache Cache>
        consteval size_t operator"" _hash()
        {
            return _Hash_bytes(Cache.arr, std::size(Cache.arr) - 1);
        }

        static_assert("test"_hash == u8"test"_hash);
        static_assert(u"test"_hash == "t\0e\0s\0t\0"_hash);
        static_assert(U"test"_hash == u"t\0e\0s\0t\0"_hash);
        static_assert(U"ab"_hash == "a\0\0\0b\0\0\0"_hash);
    } // namespace literals

} // namespace fd
