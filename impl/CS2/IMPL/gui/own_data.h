#pragma once

#include "gui/basic_data.h"
#include "gui/render/backend/own_dx11.h"
#include "gui/render/backend/own_win32.h"

namespace fd::gui
{
using own_data_dx11 = basic_own_data<own_win32_backend, own_dx11_backend>;
} // namespace fd::gui