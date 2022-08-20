module;

#include <memory>

export module fd.memory;

#define UNIQUE_PTR_USINGS             \
    using pointer         = T*;       \
    using reference       = T&;       \
    using const_reference = const T&; \
    using element_type    = T;        \
    using deleter_type    = D;        \
    using reference       = T&;

template <typename T, typename D>
struct basic_unique_ptr
{
    UNIQUE_PTR_USINGS;

  protected:
    pointer ptr_;
    [[no_unique_address]] deleter_type del_;

  private:
    constexpr void _Reset(const std::nullptr_t)
    {
        ptr_ = nullptr;
    }

    constexpr void _Reset(const pointer new_ptr)
    {
        del_(ptr_);
        ptr_ = new_ptr;
    }

  public:
    constexpr ~basic_unique_ptr()
    {
        if (!ptr_)
            return;
        _Reset(nullptr);
    }

    constexpr basic_unique_ptr(const pointer ptr = nullptr)
        : ptr_(ptr)
        , del_()
    {
    }

    template <typename D1>
    constexpr basic_unique_ptr(const pointer ptr, D1&& del)
        : ptr_(ptr)
        , del_(std::forward<D1>(del))
    {
    }

    constexpr basic_unique_ptr(const basic_unique_ptr&) = delete;

    constexpr basic_unique_ptr(basic_unique_ptr&& other)
    {
        *this = std::move(other);
    }

    constexpr void operator=(const basic_unique_ptr&) = delete;

    constexpr void operator=(basic_unique_ptr&& other)
    {
        ptr_ = other.release();
        del_ = std::move(other.del_);
    }

    constexpr void operator=(const std::nullptr_t)
    {
        _Reset(nullptr);
    }

    constexpr void operator=(const pointer ptr)
    {
        _Reset(ptr);
    }

    //----

    constexpr pointer release()
    {
        return std::exchange(ptr_, nullptr);
    }

    constexpr void reset()
    {
        _Reset(nullptr);
    }

    // swap

    constexpr pointer get() const
    {
        return ptr_;
    }

    // get_deleter

    constexpr explicit operator bool() const
    {
        return ptr_;
    }
};

template <typename T>
struct default_delete
{
    constexpr void operator()(T* ptr) const
    {
        delete ptr;
    }
};

template <typename T>
struct default_delete<T[]>
{
    constexpr void operator()(T* ptr) const
    {
        delete[] ptr;
    }
};

template <typename T, typename D = default_delete<T>>
class unique_ptr : public basic_unique_ptr<T, D>
{
    using _Base = basic_unique_ptr<T, D>;

  public:
    UNIQUE_PTR_USINGS;

    using _Base::_Base;
    using _Base::operator=;

    constexpr reference operator*() const
    {
        return *_Base::ptr_;
    }

    constexpr pointer operator->() const
    {
        return _Base::ptr_;
    }
};

template <typename T>
unique_ptr(T*) -> unique_ptr<T>;

template <typename T, typename D>
unique_ptr(T*, D&&) -> unique_ptr<T, std::remove_cvref_t<D>>;

template <typename T, typename D>
class unique_ptr<T[], D> : public basic_unique_ptr<T, D>
{
    using _Base = basic_unique_ptr<T, D>;

  public:
    UNIQUE_PTR_USINGS;

    using _Base::_Base;
    using _Base::operator=;

    constexpr reference operator[](const size_t idx) const
    {
        return _Base::ptr_[idx];
    }
};

export namespace fd
{
    using ::default_delete;
    using ::unique_ptr;
} // namespace fd
