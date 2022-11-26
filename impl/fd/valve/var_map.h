#pragma once

#include <fd/valve/vector.h>

namespace fd::valve
{
    class interpolated_var;

    struct var_map_entry
    {
        unsigned short type;
        unsigned short needs_to_interpolate; // Set to false when this var doesn't
        // need Interpolate() called on it anymore.
        void* data;
        interpolated_var* watcher;
    };

    struct var_map
    {
        vector<var_map_entry> entries;
        int interpolated_entries      = 0;
        float last_interpolation_time = 0;
    };
} // namespace fd::valve
