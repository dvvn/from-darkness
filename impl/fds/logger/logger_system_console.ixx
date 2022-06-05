module;

#include <fds/core/object.h>

export module fds.logger.system_console;
import fds.logger;

FDS_OBJECT(logger_system_console, fds::logger);

export namespace fds
{
    using ::logger_system_console;
}
