#pragma once

#include "logger.h"
#include "noncopyable.h"
#include "system_console.h"

#include <tchar.h>

namespace fd
{
using system_console_logger = basic_logger<system_console>;

#ifdef _DEBUG
struct system_console_debug_logger : system_console_logger
{
    ~system_console_debug_logger()
    {
        operator()(_T("Destroyed"));
    }

    system_console_debug_logger()
    {
        operator()(_T("Created"));
    }
};
#else
using system_console_debug_logger = fake_logger;
#endif

namespace detail
{
class basic_context : public noncopyable
{
  public:
    static system_console_logger make_logger()
    {
        return {};
    }

    static system_console_debug_logger make_debug_logger()
    {
        return {};
    }
};
} // namespace detail
} // namespace fd