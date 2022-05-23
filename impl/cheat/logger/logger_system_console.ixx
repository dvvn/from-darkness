module;

#include <cheat/core/object.h>

export module cheat.logger.system_console;
import cheat.logger;

constexpr size_t _Sys_logger_idx = 0;

export namespace cheat
{
    CHEAT_OBJECT(logger_system_console, logger, _Sys_logger_idx);
}
