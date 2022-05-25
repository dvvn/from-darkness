module;

#include <string_view>

module cheat.logger;

void logger::log(const std::string_view str)
{
    if (!active())
        return;
    log_impl(str);
}

void logger::log(const std::wstring_view str)
{
    if (!active())
        return;
    log_impl(str);
}
