#include "library.h"
#include "library_name.h"

#include <fd/tool/vector.h>

#include <boost/lambda2.hpp>

#include <windows.h>
#include <winternl.h>

#include <cassert>

#if !defined(_delayimp_h) && _MSC_VER >= 1300
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

using boost::lambda2::_1;

namespace fd
{
template <std::invocable<LDR_DATA_TABLE_ENTRY *> Filter>
static LDR_DATA_TABLE_ENTRY *find_library(Filter filter) noexcept
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

LDR_DATA_TABLE_ENTRY *find_library(void *base_address, library_cache *cache)
{
    return find_library(_1->*&LDR_DATA_TABLE_ENTRY::DllBase == base_address);
}

LDR_DATA_TABLE_ENTRY *find_library(system_string_view name, library_cache *cache)
{
    return find_library(bind(valid_library_name, _1, name));
}

LDR_DATA_TABLE_ENTRY *find_library(system_cstring name, size_t length, library_cache *cache)
{
    return find_library({name, length});
}

LDR_DATA_TABLE_ENTRY *const this_library = find_library(
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
}