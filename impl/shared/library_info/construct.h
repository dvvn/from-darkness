#pragma once

#include "library_info.h"

#include <algorithm>

namespace fd
{
LIST_ENTRY* library_info::module_list()
{
#ifdef _WIN64
    auto const mem = reinterpret_cast<TEB*>(__readgsqword(FIELD_OFFSET(NT_TIB64, Self)));
    auto const ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto const mem = reinterpret_cast<PEB*>(__readfsdword(FIELD_OFFSET(NT_TIB32, Self)));
    auto const ldr = mem->Ldr;
#endif

    return &ldr->InMemoryOrderModuleList;
}

LDR_DATA_TABLE_ENTRY_FULL* library_info::ldr_table(LIST_ENTRY* entry)
{
    return CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY_FULL, InMemoryOrderLinks);
}

library_info::library_info(wchar_t const* name, size_t const length)
{
    auto const name_last = name + length;

    auto const root_list = module_list();
    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        using std::begin;
        using std::end;
        using std::equal;

        auto const entry = ldr_table(list_entry);
        if (!entry->BaseDllName)
            continue;
        if (!equal(name, name_last, begin(entry->BaseDllName), end(entry->BaseDllName)))
            continue;

        entry_full_ = entry;
        return;
    }

    entry_full_ = nullptr;
}

template <size_t Length, class Config>
library_info::library_info(detail::basic_static_string_full<wchar_t, Length, Config> const& name)
    : library_info{name.data(), name.length()}
{
}

template <std::derived_from<library_info> Other>
library_info::library_info(Other other)
    : entry_{other.entry_}
{
}

library_info::library_info(wstring_view const name)
    : library_info{name.data(), name.length()}
{
}

library_info::library_info(wstring_view const name, wstring_view const ext)
{
    using std::data;
    using std::size;

    auto const name_length = size(name);
    auto const name_first  = data(name);
    auto const name_last   = name_first + name_length;

    auto const ext_length = size(ext);
    auto const ext_first  = data(ext);
    auto const ext_last   = ext_first + ext_length;

    auto const full_name_length = name_length + ext_length;

    auto const root_list = module_list();
    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        using std::equal;

        auto const entry = ldr_table(list_entry);
        if (!entry->BaseDllName)
            continue;
        if (full_name_length != size(entry->BaseDllName))
            continue;

        auto const entry_first = data(entry->BaseDllName);
        if (!equal(ext_first, ext_last, entry_first + name_length))
            continue;
        if (!equal(name_first, name_last, entry_first))
            continue;

        entry_full_ = entry;
        return;
    }

    entry_full_ = nullptr;
}
} // namespace fd