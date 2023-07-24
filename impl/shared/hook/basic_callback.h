#pragma once
#include "noncopyable.h"
#include "object.h"

#include <atomic>

namespace fd
{
class basic_hook_callback : public basic_object
{
    std::atomic<bool> in_use_; 

  public:
    ~basic_hook_callback() override;
    basic_hook_callback();

    void enter();
    void exit();
};

class hook_callback_call final : public noncopyable
{
    basic_hook_callback *callback_;

  public:
    ~hook_callback_call();
    hook_callback_call(basic_hook_callback *callback);

    hook_callback_call(hook_callback_call &&other) noexcept
        : callback_(other.callback_)
    {
        other.callback_ = nullptr;
    }

    hook_callback_call &operator=(hook_callback_call &&other) noexcept
    {
        using std::swap;
        swap(callback_, other.callback_);
        return *this;
    }

    basic_hook_callback const *get() const
    {
        return callback_;
    }
};
} // namespace fd