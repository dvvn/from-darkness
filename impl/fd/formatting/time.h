#pragma once

#include <fmt/chrono.h>

namespace fd
{
constexpr fmt::string_view format_time_sample = "[{:%H:%M:%S}]";

template <class Clock = std::chrono::system_clock, typename C>
void format_time(std::back_insert_iterator<C> it, typename Clock::duration time = Clock::now().time_since_epoch())
{
    fmt::format_to(it, format_time_sample, time);
}

template <class Clock = std::chrono::system_clock, typename It>
void format_time(It &it, typename Clock::duration time = Clock::now().time_since_epoch())
{
    it = fmt::format_to(it, format_time_sample, time);
}
} // namespace fd