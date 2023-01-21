#pragma once

#include <fd/string.h>

namespace fd
{
    struct basic_logs_handler
    {
        virtual ~basic_logs_handler() = default;

        virtual void write(string_view msg) const  = 0;
        virtual void write(wstring_view msg) const = 0;

        static void                set(basic_logs_handler* logger);
        static basic_logs_handler* get();
    };

    bool log_active();
    void log(string_view msg);
    void log(wstring_view msg);
    void log_unsafe(string_view msg);
    void log_unsafe(wstring_view msg);
} // namespace fd