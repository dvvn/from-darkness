module;

#include <cstdint>

export module fd.netvars.storage;
export import fd.string;

export namespace fd
{
    struct basic_netvars_storage
    {
        virtual ~basic_netvars_storage()                                                      = default;
        virtual size_t get_offset(const string_view class_name, const string_view name) const = 0;
    };

    basic_netvars_storage* netvars_storage;
} // namespace  fd
