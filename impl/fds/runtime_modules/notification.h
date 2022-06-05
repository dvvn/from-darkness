#pragma once

#include <fds/runtime_modules/notification_fwd.h>

#include <concepts>
#include <tuple>
//#include <variant>
#include <vector>

template <typename... Args>
struct rtm_notification final : basic_rtm_notification<Args...>
{
    using base_type = basic_rtm_notification<Args...>;

    using base_type::func_type;
    using storage_type = std::vector<func_type>;

    // using value_type   = std::variant<std::monostate, func_type, storage_type>;
    union
    {
        func_type    func_;
        storage_type storage_;
    };

    bool extended_;

    rtm_notification()
    {
        std::construct_at(&func_, nullptr);
        extended_ = false;
    }

    ~rtm_notification()
    {
        if (extended_)
            std::destroy_at(&storage_);
        else
            std::destroy_at(&func_);
    }

    void add(func_type&& func) override
    {
        if (extended_)
            storage_.push_back(std::move(func));
        else if (!func_)
            func_ = std::move(func_);
        else
        {
            storage_type storage;
            storage.reserve(2);
            storage.push_back(std::move(func_));
            storage.push_back(std::move(func));
            storage_ = std::move(storage);
        }
    }

    void operator()(const Args... args) const override
    {
        if (extended_)
        {
            for (auto& fn : storage_)
                std::invoke(fn, args...);
        }
        else if (func_)
        {
            std::invoke(func_, args...);
        }
    }

    bool empty() const override
    {
        return !extended_ && !func_;
    }
};

template <typename... Args>
rtm_notification<Args...> _Rtm_notification_t(const basic_rtm_notification<Args...>&)
{
    return {};
}

#define FDS_RTM_NOTIFICATION_IMPL(_NAME_) FDS_OBJECT_BIND_AUTO2(_NAME_, decltype(_Rtm_notification_t(*_NAME_)), _NAME_)

//#define FDS_RTM_NOTIFICATION_CALL(_NAME_, ...) std::invoke(*_NAME_##_impl, __VA_ARGS__)
