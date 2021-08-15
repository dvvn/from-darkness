#pragma once

#include <memory>
#include <windows.h>

namespace cheat::utl::winapi
{
    class handle_deleter
    {
    public:
        using pointer = HANDLE;

        void operator()(HANDLE ptr) const
        {
            CloseHandle(ptr);
        }
    };

    using handle = std::unique_ptr<HANDLE, handle_deleter>;

    /*class handle: public unique_ptr_ex<HANDLE>
    {
    public:
        PREVENT_COPY_FORCE_MOVE(handle);

        handle(pointer ptr);
        ~handle( );

        bool valid( ) const;
    };*/
}
