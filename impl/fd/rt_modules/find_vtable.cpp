module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

module fd.rt_modules:find_vtable;
import :find_section;
import :helpers;
import :library_info;
import fd.address;
import fd.logger;

using fd::basic_address;
using mblock = fd::mem_block;

#if 0

// mblock = {dos + header->VirtualAddress, header->SizeOfRawData}

// template<typename T>
// static mblock _Address_to_rng(const basic_address<T> addr)
//{
//	return {addr.get<uint8_t*>( ), sizeof(uintptr_t)};
// }

template <typename T>
mblock _Make_mem_block(basic_address<T>&& addr) = delete;

template <typename T>
static mblock _Make_mem_block(const basic_address<T>& addr)
{
    // return {addr.get<uint8_t*>( ), sizeof(uintptr_t)};
    return {(uint8_t*)&addr, sizeof(uintptr_t)};
}

// todo: add x64 support
static uint8_t* _Load_vtable(const mblock dot_rdata, const mblock dot_text, const basic_address<void> type_descriptor)
{
    auto       from   = dot_rdata;
    const auto search = _Make_mem_block(type_descriptor);

    for (;;)
    {
        auto mblock = from.find_block(search);
        if (mblock.empty())
            break;
        from = from.shift_to(mblock.data() + mblock.size());

        //-------------

        const basic_address<void> xr = mblock.data();

        // so if it's 0 it means it's the class we need, and not some class it inherits from
        if (const uintptr_t vtable_offset = xr.minus(sizeof(uintptr_t) * 2).deref<1>(); vtable_offset != 0)
            continue;

        // get the object locator

        const auto vtable_address = [&] {
            const auto object_locator = xr.minus(sizeof(uintptr_t) * 3);
            const auto sig            = _Make_mem_block(object_locator);
            const auto found          = dot_rdata.find_block(sig);

            const basic_address<void> addr = found.data();
            return addr + sizeof(uintptr_t);
        }();

        // check is valid offset
        if (vtable_address.value <= sizeof(uintptr_t))
            continue;

        // get a pointer to the vtable

        // convert the vtable address to an ida pattern
        const auto temp_result = [&] {
            const auto sig = _Make_mem_block(vtable_address);
            return dot_text.find_block(sig);
        }();

        if (!temp_result.empty())
            return temp_result.data();
    }

    return nullptr;
}

static mblock _Section_to_rng(const basic_address<IMAGE_DOS_HEADER> dos, const IMAGE_SECTION_HEADER* section)
{
    uint8_t* const ptr = dos + section->VirtualAddress;
    return {ptr, section->SizeOfRawData};
}

#endif

void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify)
{
    // if (!ldr_entry)
    return nullptr;

    /* const auto [dos, nt] = dos_nt(ldr_entry);

    const auto real_name = fd::make_string(".?AV", name, "@@");

    const mblock bytes        = { dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage };
    const mblock target_block = bytes.find_block(mblock(real_name.data(), real_name.size()));
    // class descriptor
    FD_ASSERT(!target_block.empty());

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(target_block.data());
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
    type_descriptor -= sizeof(uintptr_t) * 2;

    const auto dot_rdata = find_section(ldr_entry, ".rdata");
    const auto dot_text  = find_section(ldr_entry, ".text");

    const auto result = _Load_vtable(_Section_to_rng(dos, dot_rdata), _Section_to_rng(dos, dot_text), type_descriptor);
    if (notify)
        fd::library_info(ldr_entry).log("vtable", name, result);
    else
        FD_ASSERT(result != nullptr);
    return result; */
}
