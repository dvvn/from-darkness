module;

#include <fd/assert.h>

#include <fd/rt_modules/winapi_fwd.h>

module fd.rt_modules;
import :library_info;

only_true::only_true(const bool val)
{
    FD_ASSERT(val == true);
}

using namespace fd;

void on_class_found(const LDR_DATA_TABLE_ENTRY* entry, const fd::string_view raw_name, const void* addr)
{
    const auto name_begin = raw_name.find(' '); // class or struct

    const auto object_type = raw_name.substr(0, name_begin);
    const auto object      = raw_name.substr(name_begin + 1);

    library_info(entry).log(object_type, object, addr);
}

LDR_DATA_TABLE_ENTRY* current_module::data() const
{
    return find_current_library();
}

bool current_module::loaded() const
{
    return true;
}
