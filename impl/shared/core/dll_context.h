#pragma once

#include "core/basic_dll_context.h"
#include "hook/backend/minhook.h"
#include "hook/create_helper.h"

namespace fd
{
class dll_context : public basic_dll_context
{
    using hook_backend_type = hook_backend_minhook;

  protected:
    [[no_unique_address]] basic_context_data_holder<create_hook_helper<hook_backend_type>> hook_creator;
};
} // namespace fd