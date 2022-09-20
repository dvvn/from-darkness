#pragma once

#include <Windows.h>

struct IDirect3DDevice9;

namespace fd
{
    struct backend_data
    {
        HMODULE handle;
        LPCTSTR name;
        HWND hwnd;
        IDirect3DDevice9* d3d;

        ~backend_data();
        backend_data();

        backend_data(const backend_data&) = delete;

        void run();
    };
} // namespace fd
