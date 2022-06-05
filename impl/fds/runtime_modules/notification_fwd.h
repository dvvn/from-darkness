#pragma once

#include <fds/core/object.h>

#include <concepts>
#include <functional>

template <typename... Args>
struct basic_rtm_notification
{
    using func_type = std::function<void(Args...)>;

    virtual ~basic_rtm_notification() = default;

    virtual void add(func_type&& func)                = 0;
    virtual void operator()(const Args... args) const = 0;
    virtual bool empty() const                        = 0;
};

namespace std
{
    template <typename... Args>
    void invoke(const basic_rtm_notification<Args...>& notif, const Args&... args)
    {
        if (!notif.empty())
            notif(args...);
    }
} // namespace std

//__COUNTER__ broken here?
#define FDS_RTM_NOTIFICATION(_NAME_, ...) FDS_OBJECT(_NAME_, basic_rtm_notification<__VA_ARGS__>, __COUNTER__)
