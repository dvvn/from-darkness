#pragma once

namespace fd
{
    using unload_handler = void (*)();

    void unload();
    void set_unload(unload_handler fn);

    [[noreturn]] void suspend();
    [[noreturn]] void unreachable();
}