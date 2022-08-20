module;

export module fd.netvars;
export import fd.netvars.core;
import fd.address;
import fd.chars_cache;

using _Address = fd::address<const void>;

export namespace fd::netvars
{
    _Address apply_offset(const void* thisptr, const size_t offset)
    {
        return reinterpret_cast<uintptr_t>(thisptr) + offset;
    }

    template <chars_cache Table, chars_cache Item>
    _Address apply_offset(const void* thisptr)
    {
        static const auto offset = get_offset(Table.view(), Item.view());
        return apply_offset(thisptr, offset);
    }
} // namespace fd::netvars
