#pragma once

#include "core/dll_context.h"
#include "gui/native_data.h"

namespace fd
{
class dll_context_dx11 : public dll_context
{
  protected:
    [[no_unique_address]] basic_context_data_holder<gui::native_data_dx11> gui_data;
};
} // namespace fd