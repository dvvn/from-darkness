#pragma once

// #include <d3d9.h>

//  ReSharper disable once CppInconsistentNaming
struct IDirect3DDevice9;

namespace fd
{
using render_backend = IDirect3DDevice9 *;
}