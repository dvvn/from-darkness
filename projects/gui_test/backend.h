#pragma once

#include <fd/comptr.h>

#include <Windows.h>
#include <d3d9.h>

namespace fd
{
    class d3d_device9
    {
        comptr<IDirect3D9> d3d_;
        comptr<IDirect3DDevice9> device_;
        D3DPRESENT_PARAMETERS params_;

      public:
        d3d_device9();

        bool attach(const HWND hWnd);
        bool resize(const UINT w, const UINT h);
        void reset();

        operator IDirect3DDevice9*() const;
        IDirect3DDevice9* get() const;
        IDirect3DDevice9* operator->() const;
    };

    struct backend_data
    {
        d3d_device9 d3d;
        HWND hwnd;
        WNDCLASSEX info;

        ~backend_data();
        backend_data();

        backend_data(const backend_data&) = delete;

        void run();
    };
} // namespace fd
