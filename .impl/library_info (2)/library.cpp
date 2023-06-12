#include "library.h"
#include "library_name.h"

#include <fd/tool/functional.h>
#include <fd/tool/string_view.h>

#include <windows.h>
#include <winternl.h>

#include <cassert>

#if !defined(_delayimp_h) && _MSC_VER >= 1300
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace fd
{
using placeholders::_1;

template <std::invocable<system_library_entry> Filter>
static system_library_entry find_library(Filter filter) noexcept
{
#ifdef _WIN64
    auto mem = NtCurrentTeb();
    auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto mem = reinterpret_cast<PEB *>(__readfsdword(0x30));
    auto ldr = mem->Ldr;
#endif
    auto root_list = &ldr->InMemoryOrderModuleList;

    for (auto entry = root_list->Flink; entry != root_list; entry = entry->Flink)
    {
        auto table = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (filter(table))
            return table;
    }
    return nullptr;
}

#if 0
static system_library_data find_library(auto value, auto filter, basic_library_cache *cache)
{
    if (!cache)
        return find_library(filter);

    auto found = cache->find(value);
    if (!found)
    {
        found = find_library(filter);
        if (found)
            cache->store(found);
    }
    return found;
}
#endif

system_library_entry find_library(void *base_address)
{
    return find_library(_1->*&LDR_DATA_TABLE_ENTRY::DllBase == base_address);
}

system_library_entry find_library(system_string_view name)
{
    return find_library(bind(library_has_name, _1, name));
}

#if 0
system_library_data const this_library = find_library(
#if _MSC_VER >= 1300
    &__ImageBase
#else
    [] {
        MEMORY_BASIC_INFORMATION mbi;
        static int address;
        VirtualQuery(&address, &mbi, sizeof(mbi));
        return mbi.AllocationBase;
    }()
#endif
);
#endif
}