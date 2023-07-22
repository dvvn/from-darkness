#pragma once

#include "concepts.h"
#include "noncopyable.h"
#include "object.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

#include <boost/hana/append.hpp>
#include <boost/hana/tuple.hpp>

namespace fd
{
// basic_object
namespace detail
{
template <typename>
void init_once()
{
#ifdef _DEBUG
    static auto used = false;
    if (used)
        unreachable();
    used = true;
#endif
}
} // namespace detail

template <class T>
class unique_object final : public noncopyable
{
    T *object_;

  public:
    ~unique_object()
    {
        if (!object_)
            return;
        object_->~T();
    }

    /*explicit*/ unique_object(T *ifc)
        : object_(ifc)
    {
        static_assert(std::derived_from<T, basic_object>);
    }

    template <std::derived_from<T> T2 = T>
    unique_object(unique_object<T2> &&other)
        : unique_object(other.release())
    {
    }

    template <std::derived_from<T> T2 = T>
    unique_object &operator=(unique_object<T2> &&other) noexcept
    {
        if constexpr (std::swappable<T, T2>)
        {
            using std::swap;
            swap(object_, other.object_);
        }
        else
        {
            ~unique_object();
            object_ = other.release();
        }
        return *this;
    }

    T *release()
    {
        return std::exchange(object_, nullptr);
    }

    operator T *() const
    {
        return object_;
    }

    T &operator*() const
    {
        return *object_;
    }

    T *operator->() const
    {
        return object_;
    }
};

template <class T>
unique_object<T> const *operator&(unique_object<T> const &) = delete;

template <class T>
unique_object<T> *operator&(unique_object<T> &) = delete;

template <typename... Args>
using object_args_packed = boost::hana::tuple<Args...>;

template <class T, bool = complete<T>>
struct object_info;

template <class T>
struct object_info<T, true>
{
    using base        = T;
    using args_packed = void;
    using wrapped     = std::conditional_t<std::is_trivially_destructible_v<T>, T *, unique_object<T>>;
    using unwrapped   = T *;

    template <typename... Args>
    static wrapped construct(Args &&...args)
    {
        static_assert(std::derived_from<T, basic_object>);
        detail::init_once<T>();
        static uint8_t buff[sizeof(T)];
        return new (&buff) T(std::forward<Args>(args)...);
    }

    static wrapped construct(object_args_packed<>)
    {
        return construct();
    }

    template <typename... Args>
    static wrapped construct(object_args_packed<Args...> &args_packed)
    {
        return boost::hana::unpack(std::move(args_packed), [](Args... args) -> wrapped {
            return construct(static_cast<Args>(args)...);
        });
    }
};

template <class AbstractT, typename... ConstructArgs>
struct incomplete_object_info
{
    static constexpr size_t args_count = sizeof...(ConstructArgs);

    using base        = AbstractT;
    using args_packed = object_args_packed<ConstructArgs...>;
    using wrapped     = unique_object<AbstractT>;
};

#define FD_OBJECT_FN(_T_) wrapped_object<_T_> make_object(std::type_identity<_T_>, object_construct_args<_T_> args)

#define FD_OBJECT_FWD(_T_, _IFC_, ...)                                           \
    template <>                                                                  \
    struct object_info<_T_> final : incomplete_object_info<_IFC_, ##__VA_ARGS__> \
    {                                                                            \
        static wrapped construct(args_packed args_packed);                       \
    };
#define FD_OBJECT_IMPL(_T_)                                                   \
    auto object_info<_T_, false>::construct(args_packed packed_args)->wrapped \
    {                                                                         \
        return object_info<_T_, true>::construct(packed_args);                \
    }

namespace detail
{
template <size_t N, size_t Level, class Tpl, typename A, typename... Args>
auto pack_front(Tpl &&tpl, A &&arg1, Args &&...args)
{
    auto new_tpl = boost::hana::append(tpl, std::forward<A>(arg1));
    if constexpr (N == 1)
        return new_tpl;
    else
        return pack_front<N - 1, Level + 1>(new_tpl, std::forward<Args>(args)...);
}

template <size_t N, typename... Args>
auto pack_front(Args &&...args)
{
    return pack_front<N, 1>(boost::hana::tuple(), std::forward<Args>(args)...);
}

template <size_t N, class Tpl = void, typename A, typename... Args>
auto pack_back(A &&, Args &&...args)
{
    if constexpr (N != 1)
        return pack_back<N - 1, Tpl>(std::forward<Args>(args)...);
    else if constexpr (std::is_void_v<Tpl>)
        return boost::hana::tuple(std::forward<Args>(args)...);
    else
        return Tpl(std::forward<Args>(args)...);
}

} // namespace detail

template <class T, typename... Args>
auto make_object(Args &&...args)
{
    using info_t      = object_info<T>;
    using args_packed = typename info_t::args_packed;
    if constexpr (std::is_void_v<args_packed>)
        return info_t::construct(std::forward<Args>(args)...);
    else if constexpr (sizeof...(Args) == info_t::args_count)
        return info_t::construct(args_packed(std::forward<Args>(args)...));
    else
    {
        constexpr auto dont_pack = sizeof...(Args) - info_t::args_count;

        auto front_packed = detail::pack_front<dont_pack>(std::forward<Args>(args)...);
        auto packed_args  = detail::pack_back<dont_pack, args_packed>(std::forward<Args>(args)...);

        return boost::hana::unpack(
            std::move(front_packed), //
            [&packed_args]<typename... FrontArgs>(FrontArgs &&...front_args) {
                return info_t::construct(std::forward<FrontArgs>(front_args)..., std::move(packed_args));
            });
    }
}
} // namespace fd