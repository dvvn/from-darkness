module;

#include <functional>

export module fd.callback;

template <typename Fn, typename T>
Fn _Wrap_callback(T&& obj)
{
    using obj_t = decltype(obj);
    if constexpr (std::copyable<obj_t> && std::constructible_from<Fn, obj_t>)
        return std::forward<T>(obj);
    else if constexpr (std::is_lvalue_reference_v<obj_t>)
        return [&]<typename... Args>(Args... args) {
            if constexpr (std::invocable<obj_t, Args...>)
                return std::invoke(obj, args...);
            else
                return std::invoke(*obj, args...); // hack for one_instance
        };
}

template <typename... Args>
struct abstract_callback
{
    using callback_type = std::function<void(Args...)>;

    virtual ~abstract_callback() = default;

    virtual void append(callback_type&& callback) = 0;

    template <typename Fn>
    void append(Fn&& callback) requires(!std::same_as<decltype(callback), callback_type&&>)
    {
        append(_Wrap_callback<callback_type>(std::forward<Fn>(callback)));
    }

    template <typename Fn>
    void operator+=(Fn&& callback)
    {
        append(std::forward<Fn>(callback));
    }

    virtual void invoke(Args... args) const = 0;

    void operator()(Args... args) const
    {
        invoke(args...);
    }

    virtual bool empty() const = 0;
};

export namespace fd
{
    using ::abstract_callback;
}
