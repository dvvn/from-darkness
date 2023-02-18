#include <fd/log.h>
#include <fd/log_handler.h>

namespace fd
{
static basic_log_handler* _Logger = nullptr;

void set_log_handler(basic_log_handler* handler)
{
    _Logger = handler;
}

#ifndef _DEBUG
bool log_active()
{
    return _Logger;
}

void log_unsafe(const string_view msg)
{
    _Logger->write(msg);
}

void log_unsafe(const wstring_view msg)
{
    _Logger->write(msg);
}
#endif

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
    _Logger->write(msg);}

}