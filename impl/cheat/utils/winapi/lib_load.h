#pragma once

#include <memory>
#include <windows.h>

namespace utl::winapi
{
    class module_unloader
    {
    public:
        using pointer = HMODULE;

        void operator()(HMODULE ptr) const
        {
            FreeLibrary(ptr);
        }
    };

    using module_handle = std::unique_ptr<HMODULE, module_unloader>;
}
