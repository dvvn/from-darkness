#pragma once
#include <fd/logging/levels.h>

#include <fmt/format.h>

template <typename C>
struct fmt::formatter<fd::log_level, C> : formatter<basic_string_view<C>, C>
{
    auto format(fd::log_level level, auto &ctx) const -> decltype(ctx.out())
    {
        using fd::log_level;

        basic_memory_buffer<C> buff;
        auto it = std::back_inserter(buff);

        auto write = [&](char const *begin, size_t length) {
            std::copy(begin, begin + length, it);
        };

        if (level == log_level::off)
        {
            write("disabled", 8);
        }
        else
        {
            auto flag_written = false;
            auto write_flag   = [&](auto &flag, log_level val) {
                if (!have_log_level(level, val))
                    return;
                if (flag_written)
                    write(" | ", 3);
                write(std::begin(flag), std::size(flag) - 1);
                // level &= ~val;
                flag_written = true;
            };

#ifdef _DEBUG
            write_flag("debug", log_level::debug);
            if (flag_written)
                goto _TRACE;
#endif

            write_flag("info", log_level::info);
            write_flag("warning", log_level::warn);
            write_flag("error", log_level::err);
            write_flag("critical", log_level::critical);
        _TRACE:
            write_flag("trace", log_level::trace);
        }

        return formatter<basic_string_view<C>, C>::format({ buff.data(), buff.size() }, ctx);
    }
};

namespace fd
{
template <typename C>
class log_data
{
    using buffer_type = fmt::basic_memory_buffer<C>;

    log_level level_;
    buffer_type buffer_;
    void *user_data_;

  public:
    FMT_CONSTEXPR20 log_data(log_level level)
        : level_(level)
    {
    }

    void *user_data() const
    {
        return user_data_;
    }

    void set_user_data(void *user_data)
    {
        user_data_ = user_data;
    }

    FMT_CONSTEXPR20 log_level level() const
    {
        return level_;
    }

    FMT_CONSTEXPR20 auto begin() const
    {
        return buffer_.begin();
    }

    FMT_CONSTEXPR20 auto end() const
    {
        return buffer_.end();
    }

    FMT_CONSTEXPR20 auto data() const
    {
        return buffer_.data();
    }

    FMT_CONSTEXPR20 auto size() const
    {
        return buffer_.size();
    }

    FMT_CONSTEXPR20 auto out()
    {
        return std::back_inserter(buffer_);
    }

    FMT_CONSTEXPR20 void push_back(C val)
    {
        buffer_.push_back(val);
    }

    FMT_CONSTEXPR20 void append(auto &rng)
    {
        buffer_.append(std::begin(rng), std::end(rng));
    }
};
}