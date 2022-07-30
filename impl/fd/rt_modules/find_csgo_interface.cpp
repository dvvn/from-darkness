module;

#include <cstring>

#include <fd/assert.h>

#include <fd/rt_modules/winapi_fwd.h>

module fd.rt_modules:find_csgo_interface;
import :library_info;
import fd.address;
import fd.ctype;

struct interface_reg
{
    void* (*create_fn)();
    const char* name;
    const interface_reg* next;

    //--------------

    const interface_reg* find(const fd::string_view interface_name) const
    {
        for (auto reg = this; reg != nullptr; reg = reg->next)
        {
            const auto name_size = interface_name.size();
            if (std::memcmp(interface_name.data(), reg->name, name_size) != 0)
                continue;
            const auto last_char = reg->name[interface_name.size()];
            if (last_char != '\0') // partially comared
            {
                if (!fd::is_digit(last_char)) // must be looks like IfcName001
                    continue;
#ifdef _DEBUG
                const auto idx_start = reg->name + name_size;
                const auto idx_size  = fd::str_len(idx_start);
                if (idx_size > 1)
                    FD_ASSERT(fd::is_digit(idx_start + 1, idx_start + idx_size));
                if (reg->next)
                    FD_ASSERT(!reg->next->find(interface_name));
#endif
            }
            return reg;
        }

        return nullptr;
    }
};

/* void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify)
{
    using namespace fd;
    const auto create_interface = find_export(ldr_entry, "CreateInterface"_cch, notify);
    const auto ifc_addr         = find_csgo_interface(create_interface, name, nullptr);
    if (notify)
        fd::invoke(on_csgo_interface_found, ldr_entry, name, ifc_addr);
    return ifc_addr;
} */

void* find_csgo_interface(const void* create_interface_fn, const fd::string_view name, LDR_DATA_TABLE_ENTRY* const ldr_entry_for_notification)
{
    if (!create_interface_fn)
        return nullptr;

    using reg_ptr = const interface_reg*;
    using fd::basic_address;
    const reg_ptr root_reg = basic_address(create_interface_fn)./*rel32*/ jmp(0x5).plus(0x6).deref<2>();
    const auto target_reg  = root_reg->find(name);
    FD_ASSERT(target_reg != nullptr);
    const auto ifc_addr = fd::invoke(target_reg->create_fn);
    if (ldr_entry_for_notification)
        fd::library_info(ldr_entry_for_notification).log("csgo interface", name, ifc_addr);

    return ifc_addr;
}
