#pragma once
#include <fd/logging/core.h>

namespace fd
{
struct logger_registrar : virtual core_logger
{
    logger_registrar();

    static void start();
    static void stop();

};
}