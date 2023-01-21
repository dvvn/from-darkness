#pragma once

#include <fd/logger.h>

namespace fd
{
    template <typename Callback>
    class default_logs_handler final : public basic_logs_handler
    {
        Callback callback_;

      public:
        default_logs_handler(Callback callback)
            : callback_(std::move(callback))
        {
            basic_logs_handler::set(this);
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
    default_logs_handler(Callback) -> default_logs_handler<std::decay_t<Callback>>;
} // namespace fd