module;

#include <utility>

export module fd.callback.base;
export import fd.functional.invoke;

#if 0

/*import fd.one_instance;

 template <typename Fn, typename T>
Fn _Wrap_callback(T&& obj)
{
    using obj_t = decltype(obj);
    if constexpr (std::constructible_from<Fn, obj_t>)
        return std::forward<T>(obj);
    else if constexpr (std::is_lvalue_reference_v<obj_t>)
        return [&]<typename... Args>(Args&&... args) {
            return fd::invoke(obj, std::forward<Args>(args)...);
        };
} */

struct callback_info
{
    virtual ~callback_info() = default;

    virtual size_t size() const = 0;

    virtual bool empty() const
    {
        return size() == 0;
    }
};

template <typename C>
struct basic_abstract_callback : callback_info
{
    using callback_type = C;

    virtual void push_back(callback_type&& callback)  = 0;
    virtual void push_front(callback_type&& callback) = 0;

    /* template <typename Fn>
    void push_back(Fn&& callback) requires(!std::is_same_v<decltype(callback), callback_type&&>)
    {
        push_back(_Wrap_callback<callback_type>(std::forward<Fn>(callback)));
    }

    template <typename Fn>
    void push_front(Fn&& callback) requires(!std::is_same_v<decltype(callback), callback_type&&>)
    {
        push_front(_Wrap_callback<callback_type>(std::forward<Fn>(callback)));
    } */
};

template <typename Ret, typename... Args>
struct abstract_callback : basic_abstract_callback<fd::function<Ret(Args...)>>
{
    using callback_type = fd::function<Ret(Args...)>;

    virtual void operator()(Args... args) = 0;
};

template <class C>
struct abstract_callback_custom : basic_abstract_callback<C>
{
    using callback_type = C;

    virtual void operator()() = 0;
};

export namespace fd
{
    using ::abstract_callback;
    using ::abstract_callback_custom;
} // namespace fd

#endif

using fd::invoke;

template <class Buff, typename... Args>
void _Invoke_callback(Buff* buff, Args&&... args) noexcept
{
    for (auto& fn : *buff)
        invoke(fn, std::forward<Args>(args)...);
}

template <template <typename...> class Buff, class Fn>
struct basic_callback : Buff<Fn>
{
    template <typename... Args>
    void operator()(Args&&... args)
    {
        _Invoke_callback(static_cast<Buff<Fn>*>(this), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(Args&&... args) const
    {
        _Invoke_callback(static_cast<const Buff<Fn>*>(this), std::forward<Args>(args)...);
    }
};

export namespace fd
{
    using ::basic_callback;
} // namespace fd
