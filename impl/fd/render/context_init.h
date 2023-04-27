#pragma once

#include "context.h"

namespace fd
{
bool init(render_context *ctx, render_backend backend, HWND window);

// template <class T>
// bool init(render_context *ctx, T backend, HWND window)
//     requires(!std::same_as<T, render_backend>) && requires() { static_cast<render_backend>(backend); }
//{
//     return init(ctx, static_cast<render_backend>(backend), window);
// }

}