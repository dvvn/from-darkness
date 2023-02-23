#pragma once

#include <fd/log_handler.h>

namespace fd
{
template <typename Callback>
class log_handler final : public basic_log_handler
{
    Callback callback_;

  public:
    log_handler(Callback callback)
        : callback_(std::move(callback))
    {
        set_log_handler(this);
    }

    void write(string_view msg) const override
    {
        callback_(msg);
    }

    void write(wstring_view msg) const override
    {
        callback_(msg);
    }
};

template <typename Callback>
log_handler(Callback) -> log_handler<std::decay_t<Callback>>;
} // namespace fd