#pragma once

#include "concepts.h"
#include "object.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

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
class unique_object final
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

    template <std::derived_from<T> T2>
    unique_object(unique_object<T2> &&other)
        : unique_object(other.release())
    {
    }

    unique_object(unique_object &&other) noexcept
        : object_(other.release())
    {
    }

    unique_object &operator=(unique_object &&other) noexcept
    {
        using std::swap;
        swap(object_, other.object_);
        return *this;
    }

    T *release()
    {
        auto ret = object_;
        object_  = nullptr;
        return ret;
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

template <class T>
struct object_info
{
    using base        = T;
    using args_packed = void;
    using wrapped     = std::conditional_t<std::is_trivially_destructible_v<T>, T *, unique_object<T>>;
};

template <class T>
using wrapped_object = typename object_info<T>::wrapped;

template <class T>
using object_construct_args = typename object_info<T>::args_packed;

// if we put forwarded<T> inside function it alway return false
template <class T, bool Forwarded = forwarded<T>, typename... Args>
wrapped_object<T> make_object(Args &&...args)
{
    if constexpr (Forwarded)
    {
        return make_object(std::type_identity<T>(), object_construct_args<T>(std::forward<Args>(args)...));
    }
    else
    {
        static_assert(std::derived_from<T, basic_object>);
        detail::init_once<T>();
        static uint8_t buff[sizeof(T)];
        return new (&buff) T(std::forward<Args>(args)...);
    }
}

template <class T>
wrapped_object<T> make_object(object_construct_args<T> &args_packed)
{
    static_assert(!forwarded<T>);
    if constexpr (std::is_class_v<wrapped_object<T>>)
        static_assert(!std::is_trivially_destructible_v<T>); // todo: warning
    else
        static_assert(std::is_trivially_destructible_v<T>);
    return boost::hana::unpack(std::move(args_packed), []<typename... Args>(Args &&...args) -> wrapped_object<T> {
        return make_object<T>(std::forward<Args>(args)...);
    });
}

#define FD_OBJECT_FN(_T_) wrapped_object<_T_> make_object(std::type_identity<_T_>, object_construct_args<_T_> args)

#define FD_OBJECT_INFO(_T_, _IFC_, ...)                      \
    template <>                                              \
    struct object_info<_T_> final                            \
    {                                                        \
        using base        = _IFC_;                           \
        using args_packed = boost::hana::tuple<__VA_ARGS__>; \
        using wrapped     = unique_object<_IFC_>;            \
    };
#define FD_OBJECT_FWD(_T_, ...)       \
    FD_OBJECT_INFO(_T_, __VA_ARGS__); \
    FD_OBJECT_FN(_T_);

#define FD_OBJECT_IMPL(_T_)            \
    FD_OBJECT_FN(_T_)                  \
    {                                  \
        return make_object<_T_>(args); \
    }
} // namespace fd