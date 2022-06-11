module;

#include <fds/core/assert.h>
#include <fds/core/event.h>

#include <windows.h>
#include <winternl.h>

#include <cctype>
#include <string_view>

module fds.rt_modules:find_csgo_interface;
// import :find_export;
import fds.address;
import fds.event;
//import fds.chars_cache;

FDS_EVENT_BIND(on_csgo_interface_found);

struct interface_reg
{
    void* (*create_fn)();
    const char* name;
    const interface_reg* next;

    //--------------

    const interface_reg* find(const std::string_view interface_name) const
    {
        for (auto reg = this; reg != nullptr; reg = reg->next)
        {
            if (std::memcmp(reg->name, interface_name.data(), interface_name.size()) != 0)
                continue;
            const auto last_char = reg->name[interface_name.size()];
            if (last_char != '\0')
            {
                if (!std::isdigit(last_char))
                    continue;
                FDS_ASSERT(!reg->next || !reg->next->find(interface_name));
            }
            return reg;
        }

        return nullptr;
    }
};

/* void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify)
{
    using namespace fds;
    const auto create_interface = find_export(ldr_entry, "CreateInterface"_cch, notify);
    const auto ifc_addr         = find_csgo_interface(create_interface, name, nullptr);
    if (notify)
        std::invoke(on_csgo_interface_found, ldr_entry, name, ifc_addr);
    return ifc_addr;
} */

void* find_csgo_interface(const void* create_interface_fn, const std::string_view name, LDR_DATA_TABLE_ENTRY* const ldr_entry_for_notification)
{
    using reg_ptr = const interface_reg*;
    using fds::basic_address;
    const reg_ptr root_reg = basic_address(create_interface_fn)./*rel32*/ jmp(0x5).plus(0x6).deref<2>();
    const auto target_reg  = root_reg->find(name);
    FDS_ASSERT(target_reg != nullptr);
    const auto ifc_addr = std::invoke(target_reg->create_fn);
    if (ldr_entry_for_notification)
        std::invoke(on_csgo_interface_found, ldr_entry_for_notification, name, ifc_addr);
    return ifc_addr;
}
