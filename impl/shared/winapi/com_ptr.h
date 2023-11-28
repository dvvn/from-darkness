#pragma once

#include <wrl/client.h>

#include <functional>
#include <utility>

// ReSharper disable once CppInconsistentNaming
using ULONG = unsigned long;

// ReSharper disable once CppInconsistentNaming
struct IUnknown;

namespace fd::win
{
template <class T>
struct com_ptr;

namespace detail
{
template <class From, typename To>
inline constexpr bool pptr_convertible = false;

template <class T, typename To>
inline constexpr bool pptr_convertible<com_ptr<T>, To**> = std::convertible_to<T*, To*>;

template <class T, typename To>
inline constexpr bool pptr_convertible<com_ptr<T>, To* const*> = std::convertible_to<T*, To*>;

template <class T, typename To>
inline constexpr bool pptr_convertible<com_ptr<T> const, To* const*> = std::convertible_to<T const*, To*>;
} // namespace detail

template <class T>
struct com_ptr_ref
{
    using element_type   = typename T::element_type;
    using target_pointer = typename T::pointer;

  private:
    T* ptr_;

  public:
    com_ptr_ref(T* ptr)
        : ptr_(ptr)
    {
    }

    template <typename To = element_type>
    operator To*() const requires(std::convertible_to<target_pointer, To*>)
    {
        return ptr_->get();
    }

    template <typename To = element_type**>
    operator To() requires(detail::pptr_convertible<T, To>)
    {
        if constexpr (!std::is_const_v<T> && !std::is_const_v<std::remove_pointer_t<To>>)
            ptr_->release();
        return reinterpret_cast<To>(ptr_->address_of());
    }

    template <typename To = element_type* const*>
    operator To() const requires(detail::pptr_convertible<T, To>)
    {
        return reinterpret_cast<To>(ptr_->address_of());
    }

    // for IID_PPV_ARGS
    target_pointer operator*() const
    {
        return ptr_->get();
    }

    T* operator->() const
    {
        return ptr_;
    }
};

// ReSharper disable CppInconsistentNaming

template <std::derived_from<IUnknown> T>
void** IID_PPV_ARGS_Helper(com_ptr_ref<com_ptr<T>> pp) noexcept
{
    return reinterpret_cast<void**>(static_cast<IUnknown**>(pp));
}

template <std::derived_from<IUnknown> T>
void** IID_PPV_ARGS_Helper(com_ptr_ref<com_ptr<T> const> pp) noexcept
{
    return reinterpret_cast<void**>(static_cast<IUnknown**>(pp));
}

// ReSharper restore CppInconsistentNaming

template <typename T>
struct com_ptr
{
    using pointer      = T*;
    using reference    = T&;
    using element_type = T;

  private:
    pointer ptr_;

    ULONG do_release()
    {
        return ptr_->Release();
    }

    void on_copy()
    {
        ptr_->AddRef();
    }

  public:
    ~com_ptr()
    {
        if (ptr_)
            do_release();
    }

    com_ptr(nullptr_t = {})
        : ptr_(nullptr)
    {
    }

    /*template <typename P>
    explicit com_ptr(P&& ptr) noexcept(std::is_rvalue_reference_v<P&&>) requires(std::is_pointer_v<P> && std::constructible_from<pointer, P>)
        : ptr_(ptr)
    {
        if constexpr (std::is_lvalue_reference_v<P&&>)
            on_copy();
    }*/

    explicit com_ptr(pointer const& ptr) noexcept
        : ptr_(ptr)
    {
        on_copy();
    }

    explicit com_ptr(pointer&& ptr) noexcept
        : ptr_(ptr)
    {
    }

    com_ptr(pointer const& ptr, std::in_place_t) noexcept
        : com_ptr(ptr)
    {
    }

    com_ptr(pointer&& ptr, std::in_place_t) noexcept
        : com_ptr(std::move(ptr))
    {
    }

    com_ptr(com_ptr const& other)
        : com_ptr(other.ptr_)
    {
    }

    /*template <typename P>
    com_ptr& operator=(P* ptr) requires(std::assignable_from<pointer&, P*>)
    {
        if (ptr_)
            do_release();
        ptr_ = ptr;
        return *this;
    }*/

    com_ptr& operator=(com_ptr const& other)
    {
        attach(other.ptr_);
        return *this;
    }

    com_ptr(com_ptr&& other) noexcept
        : com_ptr(std::exchange(other.ptr_, nullptr))
    {
    }

    com_ptr& operator=(com_ptr&& other) noexcept
    {
        using std::swap;
        swap(ptr_, other.ptr_);
        return *this;
    }

    ULONG release()
    {
        return ptr_ ? do_release() : 0;
    }

    void reset()
    {
        if (ptr_)
        {
            do_release();
            ptr_ = nullptr;
        }
    }

    pointer get() const
    {
        return ptr_;
    }

    pointer operator->() const
    {
        return get();
    }

    /*operator pointer() const
    {
        return get();
    }*/

    reference operator*() const
    {
        return *get();
    }

    pointer* address_of() &
    {
        return &ptr_;
    }

    pointer const* address_of() const&
    {
        return &ptr_;
    }

    /*pointer* release_and_address_of()
   {
       reset();
       return &ptr_;
   }*/

    com_ptr_ref<com_ptr> operator&() &
    {
        return {this};
    }

    com_ptr_ref<com_ptr const> operator&() const&
    {
        return {this};
    }

    void attach(pointer other)
    {
        release();
        ptr_ = other;
        on_copy();
    }
};

template <typename T>
com_ptr(T*) -> com_ptr<T>;
} // namespace fd::win
