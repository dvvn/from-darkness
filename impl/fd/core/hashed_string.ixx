module;

#include <fd/core/assert.h>
#include <fd/core/utility.h>

#include <algorithm>
#include <concepts>
#include <string>

export module fd.hashed_string;
import fd.lazy_invoke;
import fd.type_name;
import fd.hash;

#define COMMA ,

#define CALL_ORIGINAL(_NAME_) Base::_NAME_(std::forward<Args>(args)...)

#define WRAP(_RET_, _NAME_, _PROXY_, ...)  \
  private:                                 \
    using Base::_NAME_;                    \
                                           \
  public:                                  \
    template <typename... Args>            \
    _RET_ _NAME_(Args&&... args)           \
    {                                      \
        const fd::lazy_invoke lazy = [=] { \
            this->_Calc_hash();            \
        };                                 \
        _PROXY_ CALL_ORIGINAL(_NAME_);     \
        __VA_ARGS__;                       \
    }

#define WRAP_VOID(_NAME_) WRAP(void, _NAME_, )

#define WRAP_THIS(_NAME_) WRAP(auto&, _NAME_, , return *this)

template <class Base, typename T, typename Q>
decltype(auto) this_or_iterator(T* thisptr, const Q& proxy_result)
{
    if constexpr (std::same_as<Q, Base>)
    {
        // return *this instead of *Base
        return *thisptr;
    }
    else
    {
        // return the iterator
        return proxy_result;
    }
}

#define WRAP_THIS_OR_ITERATOR(_NAME_) WRAP(decltype(auto), _NAME_, decltype(auto) result =, return this_or_iterator<Base>(this COMMA result))

template <class Base, template <typename> class Hasher = fd::hash>
struct hashed_string_wrapper : Base
{
    using hash_type      = /*decltype(std::invoke(std::declval<Hasher>( ), std::declval<Base>( )))*/ size_t;
    using hash_func_type = Hasher<Base>;
    using string_type    = Base;

    static_assert(std::is_empty_v<hash_func_type>, "Hasher class must be empty");

  private:
    [[no_unique_address]] hash_func_type hasher_;
    hash_type hash_;

    constexpr hash_type _Get_hash() const
    {
        return std::invoke(hasher_, *static_cast<const Base*>(this));
    }

    constexpr void _Calc_hash()
    {
        hash_ = _Get_hash();
    }

    constexpr void _Write_hash(hash_type hash)
    {
        FD_ASSERT(_Get_hash() == hash, "Incorrect hash passed");
        hash_ = hash;
    }

    template <class Base2, template <typename> class Hasher2>
    constexpr void _Try_write_hash(const hashed_string_wrapper<Base2, Hasher2>& holder)
    {
        static_assert(std::constructible_from<Base, Base2>);

        if constexpr (!same_template<Hasher, Hasher2>())
        {
            _Calc_hash();
        }
        else
        {
            using val1_t = decltype(std::declval<Base>()[0]);
            using val2_t = decltype(std::declval<Base2>()[0]);

            if constexpr (std::same_as<val1_t, val2_t>)
                hash_ = holder.hash_;
            else
                _Write_hash(holder.hash_);
        }
    }

  public:
    constexpr hashed_string_wrapper()
    {
        _Calc_hash();
    }

    template <class Base2, template <typename> class Hasher2>
    constexpr hashed_string_wrapper(const hashed_string_wrapper<Base2, Hasher2>& other) requires(std::constructible_from<Base, Base2>)
        : Base(static_cast<const Base2&>(other))
    {
        _Try_write_hash(other);
    }

    template <class Base2, template <typename> class Hasher2>
    constexpr hashed_string_wrapper(hashed_string_wrapper<Base2, Hasher2>&& other) requires(std::constructible_from<Base, Base2>)
        : Base(static_cast<Base2&&>(other))
    {
        _Try_write_hash(other);
    }

    template <class Base2>
    constexpr hashed_string_wrapper(Base2&& other) requires(std::constructible_from<Base, Base2>)
        : Base(std::forward<Base2>(other))
    {
        _Calc_hash();
    }

    template <class Base2>
    constexpr hashed_string_wrapper(Base2&& other, hash_type hash) requires(std::constructible_from<Base, Base2>)
        : Base(std::forward<Base2>(other))
    {
        _Write_hash(hash);
    }

    template <typename Itr>
    constexpr hashed_string_wrapper(Itr bg, Itr ed)
        : Base(bg, ed)
    {
        _Calc_hash();
    }

    template <typename Itr>
    constexpr hashed_string_wrapper(Itr bg, Itr ed, hash_type hash)
        : Base(bg, ed)
    {
        _Write_hash(hash);
    }

    constexpr hash_func_type hash_function() const
    {
        return hasher_;
    }

    constexpr hash_type hash() const
    {
        return hash_;
    }
};

template <class H>
concept _Hashed_string_wrapped = requires(const H h)
{
    h.hash_function();
    h.hash();
};

template <class H, class H2>
concept _Hashed_string_comparable = std::equality_comparable_with<H::string_type, H2::string_type>;

template <class H, class H2>
concept _Hashed_string_native_comparable = fd::same_template<H::hash_func_type, H2::hash_func_type>();

template <_Hashed_string_wrapped H1, _Hashed_string_wrapped H2>
constexpr bool operator==(const H1& left, const H2& right)
{
    static_assert(_Hashed_string_comparable<H1, H2>, "Incomparable string type!");
    if constexpr (_Hashed_string_native_comparable<H1, H2>)
        return left.hash() == right.hash();
    else
        return std::equal(left.begin(), left.end(), right.begin(), right.end());
}

template <_Hashed_string_wrapped H1, _Hashed_string_wrapped H2>
constexpr bool operator!=(const H1& left, const H2& right)
{
    return !(left == right);
}

template <typename Chr,
          template <typename> class Hasher = fd::hash,
          class Traits                     = std::char_traits<Chr>,
          class Base                       = hashed_string_wrapper<std::basic_string_view<Chr, Traits>, Hasher>>
struct basic_hashed_string_view : Base
{
    using Base::Base;

    WRAP_THIS(substr);

    WRAP_VOID(remove_prefix);
    WRAP_VOID(remove_suffix);

  private:
#if __cplusplus > 201703L
    using Base::swap;
#endif
};

template <typename Chr>
basic_hashed_string_view(const Chr*) -> basic_hashed_string_view<Chr>;

template <typename Chr,
          template <typename> class Hasher = fd::hash,
          class Traits                     = std::char_traits<Chr>,
          class Allocator                  = std::allocator<Chr>,
          class Base                       = hashed_string_wrapper<std::basic_string<Chr, Traits, Allocator>, Hasher>>
struct basic_hashed_string : Base
{
    using Base::Base;

    WRAP_THIS(substr);

    WRAP_THIS(assign);
    WRAP_VOID(clear);
    WRAP_THIS_OR_ITERATOR(insert);
    WRAP_THIS_OR_ITERATOR(erase);
    WRAP_VOID(push_back);
    WRAP_VOID(pop_back);
    WRAP_THIS(append);
    WRAP_THIS(operator+=);
    WRAP_THIS(replace);
    WRAP_VOID(resize);
#ifdef __cpp_lib_string_resize_and_overwrite
    WRAP_VOID(resize_and_overwrite);
#endif

  private:
    using Base::swap;
};

template <typename Chr>
basic_hashed_string(const Chr*) -> basic_hashed_string<Chr>;

#define DECLARE_IMPL(_TYPE_, _PREFIX_, _POSTFIX_) using hashed_##_PREFIX_##string##_POSTFIX_ = basic_hashed_string##_POSTFIX_##<_TYPE_>;
#define DECLARE(_TYPE_, _PREFIX_)    \
    DECLARE_IMPL(_TYPE_, _PREFIX_, ) \
    DECLARE_IMPL(_TYPE_, _PREFIX_, _view)

export namespace fd
{
    using ::basic_hashed_string;
    using ::basic_hashed_string_view;

    using ::operator==;
    using ::operator!=;

#ifdef __cpp_lib_char8_t
    DECLARE(char8_t, u8);
#endif
    DECLARE(char, );
    DECLARE(wchar_t, w);
    DECLARE(char16_t, u16);
    DECLARE(char32_t, u32);
} // namespace fd
