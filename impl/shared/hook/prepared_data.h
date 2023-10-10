#pragma once

namespace fd
{
struct prepared_hook_data
{
    void* target;
    void* replace;
    void** original;
};
} // namespace fd