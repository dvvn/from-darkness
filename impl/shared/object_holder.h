#pragma once
#ifdef _DEBUG
#include "diagnostics/fatal.h"
#endif
#include "concepts.h"
#include "noncopyable.h"
#include "object.h"
#include "preprocessor.h"
#include "functional/call_traits.h"

#include <utility>

namespace fd
{
template <class T>
class unique_object final : public noncopyable
{
    T* object_;

  public:
    ~unique_object()
    {
        if (!object_)
            return;
        object_->~T();
    }

    /*explicit*/ unique_object(T* ifc)
        : object_(ifc)
    {
        static_assert(std::derived_from<T, basic_object>);
    }

    template <std::derived_from<T> T2 = T>
    unique_object(unique_object<T2>&& other)
        : unique_object(other.release())
    {
    }

    template <std::derived_from<T> T2 = T>
    unique_object& operator=(unique_object<T2>&& other) noexcept
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

    //--

    T* release()
    {
        return std::exchange(object_, nullptr);
    }

    T* get() const
    {
        return object_;
    }

    //--

    operator T*() const&
    {
        return get();
    }

    operator T*() &&
    {
        return release();
    }

    T& operator*() const
    {
        return *get();
    }

    T* operator->() const
    {
        return get();
    }
};

template <class T>
class unique_object<T*>;

template <class T>
unique_object<T> const* operator&(unique_object<T> const&) = delete;

template <class T>
unique_object<T>* operator&(unique_object<T>&) = delete;

template <class T>
struct make_incomplete_object;

namespace detail
{
template <class T>
inline uint8_t object_info_buffer[sizeof(T)];

template <class T>
void validate_object()
{
#ifdef _DEBUG
    static_assert(std::derived_from<T, basic_object>);
    static auto used = false;
    if (used)
        unreachable();
    used = true;
#endif
}

template <class T>
struct rewrap_incomplete_object : std::type_identity<unique_object<T>>
{
};

template <class T>
using rewrap_incomplete_object_t = typename rewrap_incomplete_object<T>::type;

template <class T>
struct rewrap_incomplete_object<T*> : std::type_identity<unique_object<T>>
{
};
} // namespace detail

template <typename T, bool = complete<T>>
inline FD_CONSTEXPR_OPT uint8_t make_object = 0;

template <typename T>
inline FD_CONSTEXPR_OPT auto make_object<T, true> =
    []<typename... Args>(Args&&... args) -> std::conditional_t<std::is_trivially_destructible_v<T>, T*, unique_object<T>> {
    detail::validate_object<T>();
    return new (&detail::object_info_buffer<T>) T(std::forward<Args>(args)...);
};

template <typename T>
inline FD_CONSTEXPR_OPT auto make_object<T, false> = []<typename... Args>(Args&&... args) {
    using impl_ret = typename function_info<make_incomplete_object<T>>::return_type;
    using ret_t    = detail::rewrap_incomplete_object_t<impl_ret>;
    return std::invoke_r<ret_t>(make_incomplete_object<T>(), std::forward<Args>(args)...);
};
} // namespace fd