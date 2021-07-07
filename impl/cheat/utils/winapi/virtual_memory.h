#pragma once

#include <memory>
#include <windows.h>

namespace utl::winapi
{
    /*class virtual_mem: public unique_ptr_ex<LPVOID>
    {
    public:
        PREVENT_COPY_FORCE_MOVE(virtual_mem);

        virtual_mem(pointer ptr);
    };*/

    class virtual_mem_deleter
    {
    public:
        using pointer = LPVOID;

        void operator()(LPVOID ptr) const
        {
            VirtualFree(ptr, 0, MEM_RELEASE);
        }
    };

    using virtual_mem = std::unique_ptr<LPVOID, virtual_mem_deleter>;
}
