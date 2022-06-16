module;

#include <functional>

export module fds.callback;

// template <typename T>
// using callback_arg_t = std::conditional_t<std::is_reference_v<T>, T, std::add_lvalue_reference_t<T>>;

template <typename... Args>
struct abstract_callback
{
    using callback_type = std::function<void(Args...)>;

    virtual ~abstract_callback() = default;

    virtual void append(callback_type&& callback) = 0;

    void operator+=(callback_type&& callback)
    {
        append(std::move(callback));
    }

    virtual void invoke(Args... args) const = 0;

    void operator()(Args... args) const
    {
        invoke(args...);
    }

    virtual bool empty() const = 0;
};

export namespace fds
{
    using ::abstract_callback;
}
