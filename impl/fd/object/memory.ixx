module;

#include <memory>

export module fd.memory;

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

#define _USINGS                       \
    using pointer         = T*;       \
    using reference       = T&;       \
    using const_reference = const T&; \
    using element_type    = T;        \
    using reference       = T&;

#define _USINGS_UPTR \
    _USINGS          \
    using deleter_type = D;

template <typename T, typename D>
struct basic_unique_ptr
{
    _USINGS_UPTR;

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
        reset();
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
        _Reset(static_cast<pointer>(nullptr));
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

template <typename T, typename D = default_delete<T>>
class unique_ptr : public basic_unique_ptr<T, D>
{
    using _Base = basic_unique_ptr<T, D>;

  public:
    _USINGS_UPTR;

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
    _USINGS;

    using _Base::_Base;
    using _Base::operator=;

    constexpr reference operator[](const size_t idx) const
    {
        return _Base::ptr_[idx];
    }
};

/* template <typename T>
concept is_complete = requires
{
    sizeof(T)
}; */

template <typename T>
struct ptr_data
{
    virtual T* get()       = 0;
    virtual void destroy() = 0;
};

template <typename S>
class ref_counter
{
    S counter_;

  public:
    ref_counter()
        : counter_(0)
    {
    }

    ref_counter(const ref_counter&)            = delete;
    ref_counter& operator=(const ref_counter&) = delete;

    auto operator++()
    {
        return ++counter_;
    }

    auto operator--()
    {
        return --counter_;
    }

    auto use_count() const
    {
        decltype(const_cast<ref_counter*>(this)->operator++()) ret = counter_;
        return ret;
    }
};

template <typename T>
void destroy_at(T* ptr)
{
    if constexpr (std::is_class_v<T> || std::is_union_v<T>)
        std::destroy_at(ptr);
}

template <typename T>
void destroy_at(T& ref)
{
    destroy_at(&ref);
}

using std::construct_at;

template <typename T, typename S>
struct basic_shared_ptr
{
    _USINGS;

  private:
    struct _Data : ptr_data<T>, ref_counter<S>
    {
    };

  protected:
    _Data* data_;

    void _Construct(_Data* data)
    {
        ++(*data);
        data_ = data;
    }

  public:
    ~basic_shared_ptr()
    {
        if (!data_)
            return;
        if (data_->use_count() == 0)
            return;
        if (--(*data_) > 0)
            return;

        data_->destroy();
    }

    basic_shared_ptr()
        : data_(nullptr)
    {
    }

    template <typename T1, typename D = default_delete<T1>, typename A = std::allocator<uint8_t>>
    basic_shared_ptr(T1* ptr, D deleter = {}, A allocator = {})
    {
        class _External_data : public _Data
        {
            T1* ptr_;
            [[no_unique_address]] D deleter_;
            [[no_unique_address]] A alloc_;

          public:
            _External_data(T1* ptr, D& deleter, A& allocator)
                : ptr_(ptr)
                , deleter_(std::move(deleter))
                , alloc_(std::move(allocator))
            {
            }

            void destroy() override
            {
                if (ptr_)
                    deleter_(ptr_);
                destroy_at(deleter_);
                auto a = std::move(alloc_);
                destroy_at(static_cast<_Data*>(this));
                a.deallocate(reinterpret_cast<uint8_t*>(this), sizeof(_External_data));
            }

            pointer get() override
            {
                return ptr_;
            }
        };

        auto data = reinterpret_cast<_External_data*>(allocator.allocate(sizeof(_External_data)));
        construct_at(data, ptr, deleter, allocator);
        this->_Construct(data);
    }

    template <typename T1, typename A = std::allocator<uint8_t>>
    basic_shared_ptr(T1&& value, A allocator = {})
    {
        class _Innter_data : public _Data
        {
            std::remove_cvref_t<T1> value_;
            [[no_unique_address]] A alloc_;

          public:
            _Innter_data(T1&& value, A& allocator)
                : value_(std::forward<T1>(value))
                , alloc_(std::move(allocator))
            {
            }

            void destroy() override
            {
                destroy_at(value_);
                auto a = std::move(alloc_);
                destroy_at(static_cast<_Data*>(this));
                a.deallocate(reinterpret_cast<uint8_t*>(this), sizeof(_Innter_data));
            }

            pointer get() override
            {
                return &value_;
            }
        };

        auto data = reinterpret_cast<_Innter_data*>(allocator.allocate(sizeof(_Innter_data)));
        construct_at(data, std::forward<T1>(value), allocator);
        this->_Construct(data);
    }

    basic_shared_ptr(basic_shared_ptr&& other)
    {
        data_ = std::exchange(other.data_, nullptr);
    }

    basic_shared_ptr(const basic_shared_ptr& other)
    {
        if (!other.data_)
            return;
        _Construct(other.data_);
    }

    constexpr pointer get() const
    {
        return data_->get();
    }
};

template <typename T, typename S = size_t>
class shared_ptr : public basic_shared_ptr<T, S>
{
    using _Base = basic_shared_ptr<T, S>;

  public:
    _USINGS;

    using _Base::_Base;
    using _Base::operator=;

    reference operator*() const
    {
        return *_Base::get();
    }

    pointer operator->() const
    {
        return _Base::get();
    }
};

template <typename T>
shared_ptr(T*) -> shared_ptr<T>;
template <typename T>
shared_ptr(T&&) -> shared_ptr<std::remove_cvref_t<T>>;

template <typename T, typename S>
class shared_ptr<T[], S> : public basic_shared_ptr<T, S>
{
    using _Base = basic_shared_ptr<T, S>;

  public:
    _USINGS;

    using _Base::_Base;
    using _Base::operator=;

    reference operator[](const size_t idx) const
    {
        return _Base::get()[idx];
    }
};

export namespace fd
{
    using ::default_delete;
    using ::unique_ptr;

    using ::shared_ptr;
} // namespace fd
