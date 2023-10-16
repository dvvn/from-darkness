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
struct comptr_ref
{
    using element_type   = typename T::element_type;
    using target_pointer = typename T::pointer;

  private:
    T* ptr_;

    template <typename P>
    static constexpr bool pptr_convertible = std::convertible_to<target_pointer, std::remove_pointer_t<P>>;

  public:
    comptr_ref(T* ptr)
        : ptr_(ptr)
    {
    }

    template <typename P = element_type>
    operator P*() const requires(pptr_convertible<P>)
    {
        return ptr_->get();
    }

    template <typename P = element_type**>
    operator P() requires(pptr_convertible<P>)
    {
        if constexpr (!std::is_const_v<T> && !std::is_const_v<std::remove_pointer_t<P>>)
            ptr_->release();
        return reinterpret_cast<P>(ptr_->address_of());
    }

    template <typename P = element_type* const*>
    operator P() const requires(pptr_convertible<P>)
    {
        return reinterpret_cast<P>(ptr_->address_of());
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

    void do_reset()
    {
        do_release();
        ptr_ = nullptr;
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

    explicit comptr(nullptr_t = {})
        : ptr_(nullptr)
    {
    }

    template <typename P>
    explicit comptr(P&& ptr) noexcept(std::is_rvalue_reference_v<P&&>) requires(std::is_pointer_v<P> && std::constructible_from<pointer, P>)
        : ptr_(ptr)
    {
        if constexpr (std::is_lvalue_reference_v<P&&>)
            on_copy();
    }

    comptr(comptr const& other)
        : comptr(other.ptr_)
    {
    }

    template <typename P>
    comptr& operator=(P* ptr) requires(std::assignable_from<pointer&, P*>)
    {
        if (ptr_)
            do_release();
        ptr_ = ptr;
        return *this;
    }

    comptr& operator=(comptr const& other)
    {
        ptr_ = other.ptr_;
        on_copy();
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
            do_reset();
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
