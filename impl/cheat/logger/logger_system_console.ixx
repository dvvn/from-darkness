module;

#include <cheat/core/object.h>

export module cheat.logger.system_console;
import cheat.logger_interface;

CHEAT_OBJECT(logger_system_console, cheat::logger);

export namespace cheat
{
    using ::logger_system_console;
}
