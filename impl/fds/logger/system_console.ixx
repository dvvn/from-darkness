module;

#include <fds/core/object.h>

export module fds.logger.system_console;

struct console_writer
{
    virtual ~console_writer() = default;

    virtual void operator()(const std::string_view str)   = 0;
    virtual void operator()(const std::wstring_view wstr) = 0;
};

FDS_OBJECT(system_console_writer, console_writer);

export namespace fds
{
    using ::system_console_writer;
}
