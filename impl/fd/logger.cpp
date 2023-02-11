#include <fd/logger.h>

namespace fd
{
static basic_logs_handler* _Logger = nullptr;

void basic_logs_handler::set(basic_logs_handler* logger)
{
    _Logger = logger;
}

basic_logs_handler* basic_logs_handler::get()
{
    return _Logger;
}

bool log_active()
{
#ifdef _DEBUG
    return true;
#else
    return _Logger;
#endif
}

void log(const string_view msg)
{
#ifndef _DEBUG
    if (!_Logger)
        return;
#endif
    _Logger->write(msg);
}

void log(const wstring_view msg)
{
#ifndef _DEBUG
    if (!_Logger)
        return;
#endif
    _Logger->write(msg);
}

void log_unsafe(const string_view msg)
{
    _Logger->write(msg);
}

void log_unsafe(const wstring_view msg)
{
    _Logger->write(msg);
}
}