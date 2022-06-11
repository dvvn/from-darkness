module;

#include <functional>

export module fds.event.abstract;

template <typename... Args>
struct abstract_event
{
    using callback_type = std::function<void(Args...)>;

    virtual ~abstract_event() = default;

    virtual void append(const callback_type& callback) = 0;

    void operator+=(const callback_type& callback)
    {
        append(callback);
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
    using ::abstract_event;
}
