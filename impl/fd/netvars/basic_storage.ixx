module;

#include <cstdint>

export module fd.netvars.basic_storage;
export import fd.string;

export namespace fd
{
    struct basic_netvars_storage
    {
        virtual ~basic_netvars_storage() = default;

        virtual size_t get_netvar_offset(const string_view class_name, const string_view name) const = 0;
    };
} // namespace  fd
