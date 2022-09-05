module;

#include <memory>
#include <utility>

export module fd.smart_ptr.unique;

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
struct unique_ptr
{
    using element_type = std::remove_extent_t<T>;
    using pointer      = element_type*;
    using reference    = element_type&;
    using deleter_type = D;

  private:
    pointer ptr_;
    [[no_unique_address]] deleter_type del_;

    bool _Destroy()
    {
        const auto ok = static_cast<bool>(ptr_);
        if (ok)
            del_(ptr_);
        return ok;
    }

  public:
    constexpr ~unique_ptr()
    {
        this->_Destroy();
    }

    template <typename D1 = D>
    constexpr unique_ptr(const pointer ptr = nullptr, D1&& del = {})
        : ptr_(ptr)
        , del_(std::forward<D1>(del))
    {
    }

    constexpr unique_ptr(const unique_ptr&)            = delete;
    constexpr unique_ptr& operator=(const unique_ptr&) = delete;

    constexpr unique_ptr(unique_ptr&& other)
    {
        ptr_ = other.release();
        del_ = std::move(other.del_);
    }

    constexpr unique_ptr& operator=(unique_ptr&& other)
    {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(del_, other.del_);
        return *this;
    }

    constexpr unique_ptr& operator=(const pointer ptr)
    {
        this->_Destroy();
        ptr_ = ptr;
        return *this;
    }

    //----

    constexpr pointer release()
    {
        return std::exchange(ptr_, nullptr);
    }

    constexpr void reset()
    {
        this->_Destroy();
        ptr_ = nullptr;
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

    constexpr reference operator*() const requires(!std::is_unbounded_array_v<T>)
    {
        return *ptr_;
    }

    constexpr pointer operator->() const requires(!std::is_unbounded_array_v<T>)
    {
        return ptr_;
    }

    constexpr reference operator[](const size_t idx) const requires(std::is_unbounded_array_v<T>)
    {
        return ptr_[idx];
    }
};

template <typename T>
unique_ptr(T*) -> unique_ptr<T>;

template <typename T, typename D>
unique_ptr(T*, D&&) -> unique_ptr<T, std::remove_cvref_t<D>>;

export namespace fd
{
    using ::default_delete;
    using ::unique_ptr;
} // namespace fd
