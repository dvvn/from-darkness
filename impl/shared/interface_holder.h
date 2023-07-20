#pragma once

#include "concepts.h"
#include "interface.h"
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif

#include <boost/hana/tuple.hpp>

namespace fd
{
// basic_interface
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
class unique_interface final
{
    T *interface_;

  public:
    ~unique_interface()
    {
        if (!interface_)
            return;
        interface_->~T();
    }

    /*explicit*/ unique_interface(T *ifc)
        : interface_(ifc)
    {
        static_assert(std::derived_from<T, basic_interface>);
    }

    template <std::derived_from<T> T2>
    unique_interface(unique_interface<T2> &&other)
        : unique_interface(other.release())
    {
    }

    unique_interface(unique_interface &&other) noexcept
        : interface_(other.release())
    {
    }

    unique_interface &operator=(unique_interface &&other) noexcept
    {
        using std::swap;
        swap(interface_, other.interface_);
        return *this;
    }

    T *release()
    {
        auto ret   = interface_;
        interface_ = nullptr;
        return ret;
    }

    operator T *() const
    {
        return interface_;
    }

    T &operator*() const
    {
        return *interface_;
    }

    T *operator->() const
    {
        return interface_;
    }
};

template <class T>
unique_interface<T> const *operator&(unique_interface<T> const &) = delete;

template <class T>
unique_interface<T> *operator&(unique_interface<T> &) = delete;

template <class T>
struct interface_info
{
    using base        = T;
    using args_packed = void;
    using wrapped     = std::conditional_t<std::is_trivially_destructible_v<T>, T *, unique_interface<T>>;
};

template <class T>
using wrapped_interface = typename interface_info<T>::wrapped;

template <class T>
using interface_construct_args = typename interface_info<T>::args_packed;

// if we put forwarded<T> inside function it alway return false
template <class T, bool Forwarded = forwarded<T>, typename... Args>
wrapped_interface<T> make_interface(Args &&...args)
{
    if constexpr (Forwarded)
    {
        return make_interface(std::type_identity<T>(), interface_construct_args<T>(std::forward<Args>(args)...));
    }
    else
    {
        static_assert(std::derived_from<T, basic_interface>);
        detail::init_once<T>();
        static uint8_t buff[sizeof(T)];
        return new (&buff) T(std::forward<Args>(args)...);
    }
}

template <class T>
wrapped_interface<T> make_interface(interface_construct_args<T> &args_packed)
{
    static_assert(!forwarded<T>);
    if constexpr (std::is_class_v<wrapped_interface<T>>)
        static_assert(!std::is_trivially_destructible_v<T>); // todo: warning
    else
        static_assert(std::is_trivially_destructible_v<T>);
    return boost::hana::unpack(std::move(args_packed), []<typename... Args>(Args &&...args) -> wrapped_interface<T> {
        return make_interface<T>(std::forward<Args>(args)...);
    });
}

#define FD_INTERFACE_FN(_T_) \
    wrapped_interface<_T_> make_interface(std::type_identity<_T_>, interface_construct_args<_T_> args)

#define FD_INTERFACE_FWD(_T_, /*_TRIVIAL_,*/ _IFC_, ...)                                  \
    template <>                                                                           \
    struct interface_info<_T_> final                                                      \
    {                                                                                     \
        using base        = _IFC_;                                                        \
        using args_packed = boost::hana::tuple<__VA_ARGS__>;                              \
        using wrapped     = unique_interface<_IFC_> /*FD_INTERFACE_WRAPPED_##_TRIVIAL_*/; \
    };                                                                                    \
    FD_INTERFACE_FN(_T_);

#define FD_INTERFACE_IMPL(_T_)            \
    FD_INTERFACE_FN(_T_)                  \
    {                                     \
        return make_interface<_T_>(args); \
    }
} // namespace fd