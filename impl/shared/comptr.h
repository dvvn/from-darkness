#pragma once

#include <wrl/client.h>

#include <concepts>
#include <functional>
#include <utility>

// ReSharper disable once CppInconsistentNaming
using ULONG = unsigned long;

// ReSharper disable once CppInconsistentNaming
struct IUnknown;

namespace fd
{

template <class T>
struct comptr;

namespace detail
{
template <class From, typename To>
inline constexpr bool pptr_convertible = false;

template <class T, typename To>
inline constexpr bool pptr_convertible<comptr<T>, To**> = std::convertible_to<T*, To*>;

template <class T, typename To>
inline constexpr bool pptr_convertible<comptr<T>, To* const*> = std::convertible_to<T*, To*>;

template <class T, typename To>
inline constexpr bool pptr_convertible<comptr<T> const, To* const*> = std::convertible_to<T const*, To*>;
} // namespace detail

template <class T>
struct comptr_ref
{
    using element_type   = typename T::element_type;
    using target_pointer = typename T::pointer;

  private:
    T* ptr_;

  public:
    comptr_ref(T* ptr)
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
};

template <class T>
// ReSharper disable once CppInconsistentNaming
void** IID_PPV_ARGS_Helper(comptr_ref<T> pp) noexcept requires(std::convertible_to<T, IUnknown*>)
{
    return reinterpret_cast<void**>(static_cast<IUnknown**>(pp));
}

template <typename T>
struct comptr
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
    ~comptr()
    {
        if (ptr_)
            do_release();
    }

    comptr(nullptr_t = {})
        : ptr_(nullptr)
    {
    }

    /*template <typename P>
    explicit comptr(P&& ptr) noexcept(std::is_rvalue_reference_v<P&&>) requires(std::is_pointer_v<P> && std::constructible_from<pointer, P>)
        : ptr_(ptr)
    {
        if constexpr (std::is_lvalue_reference_v<P&&>)
            on_copy();
    }*/

    explicit comptr(pointer const& ptr) noexcept
        : ptr_(ptr)
    {
        on_copy();
    }

    explicit comptr(pointer&& ptr) noexcept
        : ptr_(ptr)
    {
    }

    comptr(pointer const& ptr, std::in_place_t) noexcept
        : comptr(ptr)
    {
    }

    comptr(pointer&& ptr, std::in_place_t) noexcept
        : comptr(std::move(ptr))
    {
    }

    comptr(comptr const& other)
        : comptr(other.ptr_)
    {
    }

    /*template <typename P>
    comptr& operator=(P* ptr) requires(std::assignable_from<pointer&, P*>)
    {
        if (ptr_)
            do_release();
        ptr_ = ptr;
        return *this;
    }*/

    comptr& operator=(comptr const& other)
    {
        std::destroy_at(this);
        std::construct_at(this, other);
        return *this;
    }

    comptr(comptr&& other) noexcept
        : comptr(std::exchange(other.ptr_, nullptr))
    {
    }

    comptr& operator=(comptr&& other) noexcept
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

    operator pointer() const
    {
        return get();
    }

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

    comptr_ref<comptr> operator&() &
    {
        return {this};
    }

    comptr_ref<comptr const> operator&() const&
    {
        return {this};
    }
};

template <typename T>
comptr(T*) -> comptr<T>;
} // namespace fd
