module;

#include <concepts>
#include <optional>

export module fd.one_instance;
import fd.functional.invoke;

using namespace fd;

template <typename T>
constexpr size_t pointers_count = 0;

template <typename T>
constexpr size_t pointers_count<T*> = 1;

template <typename T>
constexpr size_t pointers_count<T**> = pointers_count<T*> + 1;

template <size_t LastLevel = 0, typename T, size_t Level = pointers_count<T*>>
decltype(auto) _Deref(T* ptr)
{
    if constexpr (std::is_pointer_v<T> && LastLevel != Level)
        return _Deref<LastLevel, std::remove_pointer_t<T>, Level - 1>(*ptr);
    else
        return *ptr;
}

template <typename T>
bool _Null(T* ptr)
{
    if constexpr (std::is_pointer_v<T>)
    {
        const auto next_ptr = *ptr;
        return !next_ptr || _Null(next_ptr);
    }
    else
    {
        return !ptr;
    }
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
        return _Deref<1>(ptr_);
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
    return static_cast<bool>(w);
}

template <typename T, size_t Instance>
struct instance_of_getter
{
    using value_type = T;
    using reference  = value_type&;
    using pointer    = value_type*;

  private:
    value_type item_;

  public:
    template <typename... Args>
    instance_of_getter(Args&&... args) requires(std::constructible_from<value_type, Args && ...>)
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

template <typename T, size_t Instance>
struct instance_of_getter<T*, Instance>
{
    using value_type = T*;
    using pointer    = std::conditional_t<std::is_pointer_v<T>, pointer_wrapper<value_type>, value_type>;
    using reference  = decltype(*std::declval<pointer>());

  private:
    value_type item_;

  public:
    instance_of_getter(value_type item)
        : item_(item)
    {
    }

    pointer ptr() const
    {
        return item_;
    }

    reference ref() const
    {
        return *ptr();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(ptr());
    }
};

constexpr size_t _Magic_number(const size_t value)
{
    const size_t time_offsets[] = { 0, 3, 6 };
    const size_t src            = time_offsets[value % 3];
    return __TIME__[src] ^ __TIME__[7]; // XX:XX:XX 01 2 34 5 67
}

template <typename T>
struct basic_construct_helper
{
    virtual ~basic_construct_helper() = default;
    virtual T get()                   = 0;
};

template <typename Fn, typename T>
class construct_helper : public basic_construct_helper<T>
{
    Fn fn_;

  public:
    template <typename Fn1>
    construct_helper(Fn1&& fn)
        : fn_(std::forward<Fn1>(fn))
    {
    }

    T get() override
    {
        return invoke(fn_);
    }
};

template <typename T, size_t Instance>
class instance_of_impl
{
    using t_getter = instance_of_getter<T, Instance>;

    static /* __declspec(noinline) */ auto& _Buff()
    {
        static std::optional<t_getter> buff;
        return buff;
    }

    static auto& _Lazy_constructor()
    {
        static basic_construct_helper<T>* fn;
        return fn;
    }

    static bool _Initialized()
    {
        return _Buff().has_value();
    }

    template <typename... Args>
    static decltype(auto) _Construct(Args&&... args)
    {
        if constexpr (sizeof...(Args) == 0 && std::constructible_from<t_getter, T>)
        {
            auto lazy = _Lazy_constructor();
            if (lazy)
                return _Buff().emplace(lazy->get());
        }

        if constexpr (std::constructible_from<t_getter, Args&&...>)
            return _Buff().emplace(std::forward<Args>(args)...);

        return *_Buff(); // return empty object to force error
    }

    static auto& _Get()
    {
        static const auto once = [] {
            if (!_Initialized())
                _Construct();
            return _Magic_number(Instance);
        }();
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
    decltype(auto) construct(Args&&... args) const
    {
        return _Construct(std::forward<Args>(args)...).ref();
    }

    template <typename Fn>
    size_t construct_lazy(Fn&& fn) const
    {
        static_assert(std::convertible_to<decltype(fn()), T>);
        static construct_helper<std::remove_cvref_t<Fn>, T> helper(std::forward<Fn>(fn));
        auto& lazy = _Lazy_constructor();
        lazy       = static_cast<basic_construct_helper<T>*>(&helper);
        return _Magic_number(Instance);
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
        if (_Initialized())
            return false;
        if constexpr (std::is_pointer_v<value_type>)
        {
            if (!_Get())
                return false;
        }
        return true;
    }

    template <typename... Args>
    decltype(auto) operator()(Args&&... args) const
    {
        return invoke(_Get().ref(), std::forward<Args>(args)...);
    }
};

template <typename T>
concept complete_type = requires { sizeof(T); };

using std::type_identity;

template <typename T>
constexpr auto _Object_type()
{
    if constexpr (!complete_type<T>)
        return type_identity<T*>();
    else if constexpr (std::is_abstract_v<T>)
        return type_identity<T*>();
    else
        return type_identity<T>();
}

template <typename T>
using _Object_type_t = typename decltype(_Object_type<T>())::type;

export namespace fd
{
    using ::complete_type;
    using ::type_identity;

    using ::instance_of_getter;
    using ::instance_of_impl;

    template <typename T, size_t Instance = 0>
    constexpr instance_of_impl<_Object_type_t<T>, Instance> instance_of;

    template <typename T, size_t Instance>
    constexpr instance_of_impl<T, Instance> instance_of<type_identity<T>, Instance>;

} // namespace fd
