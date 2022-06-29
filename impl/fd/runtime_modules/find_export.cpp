module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <functional>
#include <string_view>

module fd.rt_modules:find_export;
import :helpers;
import fd.address;
import fd.logger;

void* find_export(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify)
{
    if (!ldr_entry)
        return nullptr;
    // base address
    const auto [dos, nt] = fd::dos_nt(ldr_entry);

    // get export data directory.
    const auto& data_dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    FD_ASSERT(data_dir.VirtualAddress, "Current module doesn't have the virtual address!");

    // get export export_dir.
    const fd::basic_address<IMAGE_EXPORT_DIRECTORY> export_dir = dos + data_dir.VirtualAddress;

    // names / funcs / ordinals ( all of these are RVAs ).
    uint32_t* const names = dos + export_dir->AddressOfNames;
    uint32_t* const funcs = dos + export_dir->AddressOfFunctions;
    uint16_t* const ords  = dos + export_dir->AddressOfNameOrdinals;

    void* export_ptr = nullptr;

    // iterate names array.
    for (size_t i = 0; i < export_dir->NumberOfNames; ++i)
    {
        const char* export_name = dos + names[i];
        if (!export_name)
            continue;
        if (std::memcmp(export_name, name.data(), name.size()) != 0)
            continue;
        if (export_name[name.size()] != '\0')
            continue;

        /*
         if (export_addr.cast<uintptr_t>() >= reinterpret_cast<uintptr_t>(export_directory)
            && export_addr.cast<uintptr_t>() < reinterpret_cast<uintptr_t>(export_directory) + data_directory->Size)
         */

        // if (export_ptr < export_dir || export_ptr >= memory_block(export_dir, data_dir.Size).addr( ))
        const auto temp_export_ptr = dos + funcs[ords[i]];
        if (temp_export_ptr < export_dir || temp_export_ptr >= export_dir + data_dir.Size)
        {
            export_ptr = temp_export_ptr;
            break;
        }

        FD_ASSERT_UNREACHABLE("Forwarded export detected");
#if 0
		// get forwarder string.
		const std::string_view fwd_str = export_ptr.get<const char*>( );

		// forwarders have a period as the delimiter.
		const auto delim = fwd_str.find_last_of('.');
		if(delim == fwd_str.npos)
			continue;

		using namespace std::string_view_literals;
		// get forwarder mod name.
		const info_string::fixed_type fwd_module_str = nstd::append<std::wstring>(fwd_str.substr(0, delim), L".dll"sv);

		// get real export ptr ( recursively ).
		const auto target_module = std::ranges::find_if(*all_modules, [&](const info& i)
		{
			return i.name == fwd_module_str;
		});
		if(target_module == all_modules->end( ))
			continue;

		// get forwarder export name.
		const auto fwd_export_str = fwd_str.substr(delim + 1);

		try
		{
			auto& exports = target_module->exports( );
			auto fwd_export_ptr = exports.at(fwd_export_str);

			this->emplace(export_name, fwd_export_ptr);
		}
		catch(std::exception)
		{
		}
#endif
    }

    if (notify)
        std::invoke(fd::logger, "export found (WIP)"); // ldr_entry, name, export_ptr
    return export_ptr;
}
