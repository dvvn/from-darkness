#include <fd/log_init.h>

namespace fd::log
{
#ifdef _DEBUG
static auto _DefaultLogger = [] {
    auto backup = spdlog::default_logger();
    spdlog::set_default_logger(nullptr);
    return backup;
}();
#endif

template <typename T>
static void _init(T&& idt)
{
#ifdef _DEBUG
    if (_DefaultLogger)
        spdlog::set_default_logger(std::move(_DefaultLogger));
#endif

    if (!idt.pattern.empty())
        spdlog::set_pattern(std::move(idt.pattern));

    if (idt.backtrace)
        spdlog::enable_backtrace(idt.backtrace);
    else
        spdlog::disable_backtrace();
}

void init(init_data&& idt)
{
    _init(std::move(idt));
}

void init(const init_data& idt)
{
    _init(idt);
}
}