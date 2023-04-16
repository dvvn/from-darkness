#pragma once

#include <fmt/chrono.h>

namespace fd
{
template <class Clock = std::chrono::system_clock, typename It>
void format_time(It &it, typename Clock::duration time = Clock::now().time_since_epoch())
{
    it = fmt::format_to(it, "[{:%H:%M:%S}]", time);
}
} // namespace fd
