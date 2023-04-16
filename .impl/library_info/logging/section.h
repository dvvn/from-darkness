#pragma once

#include <fd/logging/logger.h>

namespace fd
{
struct library_section_logger : logger<log_level::err>
{
};
} // namespace fd
