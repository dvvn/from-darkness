#pragma once

#include "gui/basic_data.h"
#include "gui/render/backend/native_dx11.h"
#include "gui/render/backend/native_win32.h"

namespace fd::gui
{
using native_data_dx11 = basic_data<native_win32_backend, native_dx11_backend>;
} // namespace fd::gui