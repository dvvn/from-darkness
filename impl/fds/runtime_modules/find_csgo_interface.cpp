module;

#include <fds/core/assert.h>
#include <fds/runtime_modules/notification.h>

#include <windows.h>
#include <winternl.h>

#include <cctype>
#include <string_view>

module fds.rt_modules:find_csgo_interface;
import fds.address;

FDS_RTM_NOTIFICATION_IMPL(on_csgo_interface_found);

struct interface_reg
{
    void* (*create_fn)();
    const char* name;
    interface_reg* next;
};

static interface_reg* _Find_interface(const std::string_view name, interface_reg* const first, interface_reg* const last = nullptr)
{
    for (auto reg = first; reg != last; reg = reg->next)
    {
        if (std::memcmp(reg->name, name.data(), name.size()) != 0)
            continue;
        const auto last_char = reg->name[name.size()];
        if (last_char == '\0' || std::isdigit(last_char))
            return reg;
    }

    return nullptr;
}

void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify)
{
    using namespace fds;
    const auto create_interface = find_export(ldr_entry, "CreateInterface"_cch, notify);
    const auto ifc_addr         = find_csgo_interface(create_interface, name, false);
    if (notify)
        std::invoke(on_csgo_interface_found, ldr_entry, name, ifc_addr);
    return ifc_addr;
}

void* find_csgo_interface(const void* create_interface_fn, const std::string_view name, const bool notify)
{
    using ifc_reg_ptr = interface_reg* const;
    using namespace fds;
    const basic_address addr = create_interface_fn;
    ifc_reg_ptr root_reg     = addr./*rel32*/ jmp(0x5).plus(0x6).deref<2>();
    ifc_reg_ptr target_reg   = _Find_interface(name, root_reg);
    fds_assert(target_reg != nullptr);
    fds_assert(_Find_interface(name, target_reg->next) == nullptr);
    const auto ifc_addr = std::invoke(target_reg->create_fn);
    if (notify)
        std::invoke(on_csgo_interface_found, L"unknown"_cch, name, ifc_addr);
    return ifc_addr;
}
