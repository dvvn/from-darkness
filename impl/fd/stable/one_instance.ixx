module;

#include <concepts>
#include <memory>
#include <optional>
#include <utility>

export module fd.one_instance;
import fd.functional;

template <typename T>
constexpr size_t _Pointers_count()
{
    if constexpr (std::is_pointer_v<T>)
        return 1 + _Pointers_count<std::remove_pointer_t<T>>();
    else
        return 0;
}

template <typename T>
constexpr size_t pointers_count_v = _Pointers_count<T>();

template <typename T, size_t StopOn = 0>
decltype(auto) _Deref(T* ptr, const std::in_place_index_t<StopOn> stop_on = std::in_place_index<StopOn>)
{
    constexpr auto ptrsc = pointers_count_v<T*>;
    if constexpr (ptrsc <= StopOn)
        return ptr;
    else if constexpr (ptrsc == 1)
        return *ptr;
    else
        return _Deref(*ptr, stop_on);
}

template <typename T>
bool _Null(T* ptr)
{
    if (!ptr)
        return true;

    constexpr auto ptrsc = pointers_count_v<T*>;
    if constexpr (ptrsc > 1)
        return _Null(*ptr);
    else
        return false;
}

template <typename T>
class pointer_wrapper
{
    T ptr_;

  public:
    T _Get() const
    {
        return ptr_;
    }

    pointer_wrapper(T ptr)
        : ptr_(ptr)
    {
    }

    T operator->() const
    {
        return ptr_;
    }

    auto& operator*() const
    {
        return *ptr_;
    }

    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }
};

template <typename T>
class pointer_wrapper<T**>
{
    using value_type = T**;

    value_type ptr_;

  public:
    value_type _Get() const
    {
        return ptr_;
    }

    pointer_wrapper(value_type ptr)
        : ptr_(ptr)
    {
    }

    auto operator->() const
    {
        return _Deref(ptr_, std::in_place_index<1>);
    }

    auto& operator*() const
    {
        return _Deref(ptr_);
    }

    explicit operator bool() const
    {
        return !_Null(ptr_);
    }
};

template <typename T>
bool operator==(const pointer_wrapper<T> w, std::nullptr_t)
{
    return !w;
}

template <typename T>
struct instance_of_getter
{
    using value_type = T;
    using reference  = value_type&;
    using pointer    = value_type*;

  private:
    value_type item_;

  public:
    template <typename... Args>
    instance_of_getter(Args&&... args) requires(std::constructible_from<value_type, decltype(args)...>)
        : item_(std::forward<Args>(args)...)
    {
    }

    reference ref()
    {
        return item_;
    }

    pointer ptr()
    {
        return &item_;
    }
};

template <typename T>
struct instance_of_getter<T*>
{
    using value_type = T*;
    using pointer    = std::conditional_t<std::is_pointer_v<T>, pointer_wrapper<value_type>, value_type>;
    using reference  = decltype(*std::declval<pointer>());

  private:
    value_type item_;

    template <typename Q>
    void _Construct(Q item)
    {
        if constexpr (std::convertible_to<Q, value_type>)
            item_ = static_cast<value_type>(item);
        else if constexpr (std::convertible_to<decltype(&*item), value_type>)
            _Construct(&*item);
        else if constexpr (std::convertible_to<decltype(&item), value_type>)
            _Construct(&item);
        else
            static_assert(std::_Always_false<Q>, "Unknown item type!");
    }

  public:
    template <size_t Instance>
    instance_of_getter(const std::in_place_index_t<Instance>);

    instance_of_getter(const value_type item)
    {
        _Construct(item);
    }

    instance_of_getter(std::nullptr_t np)
    {
        _Construct(np);
    }

    reference ref() const
    {
        return *ptr();
    }

    pointer ptr() const
    {
        return item_;
    }
};

template <typename T>
bool operator==(const instance_of_getter<T*> getter, std::nullptr_t)
{
    return getter.ptr() == nullptr;
}

constexpr size_t _Magic_number(const size_t value)
{
    const size_t time_offsets[] = { 0, 3, 6 };
    const size_t src            = time_offsets[value % 3];
    return __TIME__[src] ^ __TIME__[7]; // XX:XX:XX 01 2 34 5 67
}

template <typename T, size_t Instance>
class instance_of_impl
{
    using t_getter = instance_of_getter<T>;

    static __declspec(noinline) auto& _Buff()
    {
        static std::optional<t_getter> buff;
        return buff;
    }

    static bool _Initialized()
    {
        return _Buff().has_value();
    }

    template <typename... Args>
    static auto& _Construct(Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0 || !std::constructible_from<t_getter, std::in_place_index_t<Instance>>)
            return _Buff().emplace(std::forward<Args>(args)...);
        else
            return _Buff().emplace(std::in_place_index<Instance>);
    }

    static auto& _Get()
    {
        if constexpr (std::default_initializable<t_getter> || std::constructible_from<t_getter, std::in_place_index_t<Instance>>)
        {
            static const auto once = [] {
                if (!_Initialized())
                    _Construct();
                return _Magic_number(Instance);
            }();
        }
        return *_Buff();
    }

  public:
    using value_type = T;

    bool initialized() const
    {
        return _Initialized();
    }

    auto& operator*() const
    {
        return _Get().ref();
    }

    auto operator->() const
    {
        return _Get().ptr();
    }

    auto operator&() const
    {
        return _Get().ptr();
    }

    template <typename... Args>
    auto& construct(Args&&... args) const
    {
        return _Construct(std::forward<Args>(args)...).ref();
    }

    void destroy() const
    {
        _Buff().reset();
    }

    template <std::same_as<size_t> T> // fake explicit
    consteval operator T() const
    {
        return Instance;
    }

    explicit operator bool() const
    {
        if (!_Initialized())
            return false;
        if constexpr (std::is_pointer_v<value_type>)
        {
            if (_Get() == nullptr)
                return false;
        }
        return true;
    }
};

template <class T, size_t Instance, typename... Args>
decltype(auto) invoke(const instance_of_impl<T, Instance> inst, Args&&... args)
{
    return fd::invoke(*inst, std::forward<Args>(args)...);
}

using fd::invoke;

export namespace fd
{
    using ::invoke;

    template <typename T, typename... Args>
    concept invocable = requires(T&& obj, Args&&... args) // override previous concept
    {
        invoke(std::forward<T>(obj), std::forward<Args>(args)...);
    };

    using ::instance_of_getter;

    /* template <typename T, size_t Instance>
    bool operator==(const instance_of_impl<T*, Instance> inst, std::nullptr_t)
    {
        return inst.initialized() && inst.ptr() == nullptr;
    } */

    template <typename T, size_t Instance = 0>
    constexpr instance_of_impl<T, Instance> instance_of;

    using ::instance_of_impl;
} // namespace fd
