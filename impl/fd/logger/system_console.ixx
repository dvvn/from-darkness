module;

#include <fd/object.h>

export module fd.system_console;
export import fd.string;

struct console_writer
{
    virtual ~console_writer() = default;

    virtual void operator()(const fd::string_view str)   = 0;
    virtual void operator()(const fd::wstring_view wstr) = 0;
};

FD_OBJECT(system_console_writer, console_writer);

export namespace fd
{
    using ::system_console_writer;
}
