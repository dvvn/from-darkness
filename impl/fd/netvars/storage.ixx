module;

#include <fd/object.h>

#include <sstream>
#include <vector>

export module fd.netvars.storage;
import fd.valve.client_class;
import fd.valve.data_map;
export import fd.string;

using fd::string_view;
using fd::wstring;

struct logs_data
{
    ~logs_data();
    logs_data();

    wstring dir;

    struct
    {
        wstring name;
        wstring extension;
    } file;

    size_t indent;
    char filler;

    std::ostringstream buff;
};

struct classes_data
{
    ~classes_data();
    classes_data();

    wstring dir;

    struct file_info
    {
        wstring name;
        std::ostringstream buff;
    };

    std::vector<file_info> files;
};

using fd::valve::client_class;
using fd::valve::data_map;

struct storage
{
    virtual ~storage() = default;

    virtual void iterate_client_class(const client_class* root_class) = 0;
    virtual void iterate_datamap(const data_map* root_map)            = 0;
    virtual void store_handmade_netvars()                             = 0;

    virtual void log_netvars(logs_data& data)         = 0;
    virtual void generate_classes(classes_data& data) = 0;

    virtual size_t get_netvar_offset(const string_view class_name, const string_view table_name, const string_view name) const = 0;
};

FD_OBJECT(netvars_storage, storage);

export namespace fd
{
    using netvar_logs_data    = logs_data;
    using netvar_classes_data = classes_data;

    using ::netvars_storage;
} // namespace fd
