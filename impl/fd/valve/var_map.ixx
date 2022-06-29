module;

export module fd.valve.var_map;
export import fd.valve.vector;

using namespace fd::valve;

class interpolated_var;

struct var_map
{
    struct entry_type
    {
        unsigned short type;
        unsigned short needs_to_interpolate; // Set to false when this var doesn't
        // need Interpolate() called on it anymore.
        void* data;
        interpolated_var* watcher;
    };

    vector<entry_type> entries;
    int interpolated_entries      = 0;
    float last_interpolation_time = 0;
};

export namespace fd::valve
{
    using ::var_map;
}
