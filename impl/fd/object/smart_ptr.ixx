module;

#include <memory>
#include <utility>

export module fd.smart_ptr;

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
struct unique_ptr;

template <typename T, typename S = size_t>
struct weak_ptr;

template <typename T, typename S = size_t>
struct shared_ptr;

template <typename T, typename D>
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

/* template <typename T>
concept is_complete = requires
{
    sizeof(T)
}; */

template <typename T>
struct basic_shared_controller
{
    virtual T* get()           = 0;
    virtual void destroy()     = 0;
    virtual void delete_self() = 0;
};

template <typename S>
class ref_counter
{
    S counter_;

  public:
    using size_type = decltype([] {
        S dummy_;
        return dummy_++;
    }());

    ref_counter()
        : counter_(0)
    {
    }

    ref_counter(const ref_counter&)            = delete;
    ref_counter& operator=(const ref_counter&) = delete;

    size_type operator++()
    {
        return ++counter_;
    }

    size_type operator--()
    {
        return --counter_;
    }

    size_type count() const
    {
        return counter_;
    }
};

template <typename T, class Alloc, typename... Args>
T* make_ptr(Alloc&& allocator, Args&&... args)
{
    auto ptr = reinterpret_cast<T*>(allocator.allocate(sizeof(T)));
    std::construct_at(ptr, std::forward<Args>(args)..., std::forward<Alloc>(allocator));
    return ptr;
}

template <typename Base, class Alloc, typename T>
void destroy_ptr(Alloc allocator, T* ptr)
{
    std::destroy_at(static_cast<Base*>(ptr));
    allocator.deallocate(reinterpret_cast<uint8_t*>(ptr), sizeof(T));
}

template <typename T, typename S>
struct shared_controller : basic_shared_controller<T>
{
    ref_counter<S> strong, weak;

    void _Destroy_strong()
    {
        if (--strong > 0)
            return;
        this->destroy();
        if (weak.count() == 0)
            this->delete_self();
    }

    void _Destroy_weak()
    {
        if (--weak > 0)
            return;
        if (strong.count() == 0)
            this->delete_self();
    }
};

enum class shared_type : uint8_t
{
    strong,
    weak
};

template <shared_type Type>
struct shared_updater;

template <>
struct shared_updater<shared_type::strong>
{
    template <class D>
    static void construct(D*& data, D* new_data)
    {
#ifdef _DEBUG
        if (!new_data)
            throw std::bad_alloc();
#endif

        ++new_data->strong;
        data = new_data;
    }

    template <class D>
    static void destroy(D* data)
    {
        if (data)
            data->_Destroy_strong();
    }
};

template <>
struct shared_updater<shared_type::weak>
{
    template <class D>
    static void construct(D*& data, D* new_data)
    {
#ifdef _DEBUG
        if (!new_data || new_data->strong.count() == 0)
            throw std::bad_weak_ptr();
#endif
        ++new_data->weak;
        data = new_data;
    }

    template <class D>
    static void destroy(D* data)
    {
        if (data)
            data->_Destroy_weak();
    }
};

struct shared_unpacker
{
    template <class H>
    auto operator()(const H& owner) const
    {
        return owner.ctrl_;
    }
};

template <typename T, typename S, shared_type Type>
struct shared_owner
{
    using element_type = std::remove_extent_t<T>;
    using pointer      = element_type*;
    using reference    = element_type&;

  private:
    using _Controller = shared_controller<element_type, S>;

    friend struct shared_unpacker;

    _Controller* ctrl_;
    [[no_unique_address]] shared_unpacker unpacker_;

  public:
    shared_owner()
        : ctrl_(nullptr)
    {
    }

    void _Construct(_Controller* new_data)
    {
        shared_updater<Type>::construct(ctrl_, new_data);
    }

    template <shared_type Type2>
    void _Construct(const shared_owner<T, S, Type2>& holder)
    {
        _Construct(unpacker_(holder));
    }

    void _Destroy()
    {
        shared_updater<Type>::destroy(ctrl_);
    }

    void _Swap(shared_owner& from)
    {
        using std::swap;
        swap(ctrl_, from.ctrl_);
    }

    // swap

    void reset()
    {
        this->_Destroy();
        ctrl_ = nullptr;
    }

    pointer get() const
    {
        return ctrl_->get();
    }

    // get_deleter

    explicit operator bool() const
    {
        return ctrl_;
    }

    reference operator*() const requires(!std::is_unbounded_array_v<T>)
    {
        return *this->get();
    }

    pointer operator->() const requires(!std::is_unbounded_array_v<T>)
    {
        return this->get();
    }

    reference operator[](const size_t idx) const requires(std::is_unbounded_array_v<T>)
    {
        return this->get()[idx];
    }

    typename ref_counter<S>::size_type use_count() const
    {
        return !ctrl_ ? 0 : ctrl_->strong.count();
    }
};

template <typename T, typename S>
struct weak_ptr : shared_owner<T, S, shared_type::weak>
{
    using element_type = std::remove_extent_t<T>;
    using shared_type  = shared_ptr<T, S>;

    ~weak_ptr()
    {
        this->_Destroy();
    }

    weak_ptr() = default;

    weak_ptr(const shared_type& other);
    weak_ptr(shared_type&& other) = delete;

    weak_ptr(const weak_ptr& other)
    {
        this->_Construct(other);
    }

    weak_ptr(weak_ptr&& other)
    {
        this->_Swap(other);
    }

    weak_ptr& operator=(const shared_type& other)
    {
        this->_Destroy();
        this->_Construct(other);
        return *this;
    }

    weak_ptr& operator=(shared_type&& other) = delete;

    weak_ptr& operator=(const weak_ptr& other)
    {
        this->_Destroy();
        this->_Construct(other);
        return *this;
    }

    weak_ptr& operator=(weak_ptr&& other)
    {
        this->_Swap(other);
        return *this;
    }

    bool expired() const
    {
        return this->use_count() == 0;
    }

    shared_type lock() const;
};

template <typename T, typename S>
struct shared_ptr : shared_owner<T, S, shared_type::strong>
{
    using element_type = std::remove_extent_t<T>;
    using pointer      = element_type*;

    using weak_type = weak_ptr<T, S>;

  private:
    using _Controller = shared_controller<T, S>;

  public:
    ~shared_ptr()
    {
        this->_Destroy();
    }

    shared_ptr() = default;

    template <typename T1, typename D = default_delete<T1>, typename A = std::allocator<uint8_t>>
    shared_ptr(T1* ptr, D deleter = {}, A allocator = {})
    {
        class _External_data : public _Controller
        {
            unique_ptr<T1, D> ptr_;
            [[no_unique_address]] A alloc_;

          public:
            _External_data(T1* ptr, D& deleter, A& allocator)
                : ptr_(ptr, std::move(deleter))
                , alloc_(std::move(allocator))
            {
            }

            void destroy() override
            {
                std::destroy_at(&ptr_);
            }

            void delete_self() override
            {
                destroy_ptr<_Controller>(std::move(alloc_), this);
            }

            pointer get() override
            {
                return ptr_;
            }
        };

        this->_Construct(make_ptr<_External_data>(allocator, ptr, deleter));
    }

    template <typename T1, typename A = std::allocator<uint8_t>>
    shared_ptr(T1&& value, A allocator = {})
    {
        class _Innter_data : public _Controller
        {
            using value_type = std::remove_cvref_t<T1>;

            value_type value_;
            [[no_unique_address]] A alloc_;

          public:
            _Innter_data(T1&& value, A& allocator)
                : value_(std::forward<T1>(value))
                , alloc_(std::move(allocator))
            {
            }

            void destroy() override
            {
                if constexpr (std::is_class_v<value_type> || std::is_union_v<value_type>)
                    std::destroy_at(&value_);
            }

            void delete_self() override
            {
                destroy_ptr<_Controller>(std::move(alloc_), this);
            }

            pointer get() override
            {
                return &value_;
            }
        };

        this->_Construct(make_ptr<_Innter_data>(allocator, std::forward<T1>(value)));
    }

    shared_ptr(shared_ptr&& other)
    {
        this->_Swap(other);
    }

    shared_ptr(const shared_ptr& other)
    {
        this->_Construct(other);
    }

    shared_ptr& operator=(const shared_ptr& other)
    {
        this->_Destroy();
        this->_Construct(other);
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other)
    {
        this->_Swap(other);
        return *this;
    }
};

template <typename T, typename S>
weak_ptr<T, S>::weak_ptr(const shared_type& other)
{
    this->_Construct(other);
}

template <typename T, typename S>
auto weak_ptr<T, S>::lock() const -> shared_type
{
    shared_type tmp;
    tmp._Construct(*this);
    return tmp;
}

template <typename T>
shared_ptr(T*) -> shared_ptr<T>;
template <typename T>
shared_ptr(T&&) -> shared_ptr<std::remove_cvref_t<T>>;

template <typename T, typename S>
weak_ptr(const shared_ptr<T, S>&) -> weak_ptr<T, S>;
template <typename T, typename S>
shared_ptr(const weak_ptr<T, S>&) -> shared_ptr<T, S>;

export namespace fd
{
    using ::default_delete;

    using ::shared_ptr;
    using ::unique_ptr;
    using ::weak_ptr;
} // namespace fd
