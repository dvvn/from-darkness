module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <tuple>

module fd.rt_modules:find_library;
import :library_info;
import fd.chars_cache;
import fd.logger;

constexpr auto on_library_found = [](const auto library_name, const LDR_DATA_TABLE_ENTRY* entry) {
    fd::invoke(fd::logger, L"{} -> found! ({:#X})", library_name, reinterpret_cast<uintptr_t>(entry));
};

static auto _Get_ldr()
{
    const auto mem =
#if defined(_M_X64) || defined(__x86_64__)
        NtCurrentTeb();
    FD_ASSERT(mem != nullptr, "Teb not found");
    return mem->ProcessEnvironmentBlock->Ldr;
#else
        reinterpret_cast<PEB*>(__readfsdword(0x30));
    FD_ASSERT(mem != nullptr, "Peb not found");
    return mem->Ldr;
#endif
}

template <typename Fn>
static LDR_DATA_TABLE_ENTRY* _Find_library(Fn comparer)
{
    const auto ldr  = _Get_ldr();
    // get module linked list.
    const auto list = &ldr->InMemoryOrderModuleList;
    // iterate linked list.
    for (auto it = list->Flink; it != list; it = it->Flink)
    {
        // get current entry.
        const auto ldr_entry = CONTAINING_RECORD(it, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (!ldr_entry)
            continue;

        const auto [dos, nt] = fd::dos_nt(ldr_entry);
        if (!comparer(ldr_entry, dos, nt))
            continue;

        return ldr_entry;
    }

    return nullptr;
}

LDR_DATA_TABLE_ENTRY* find_library(const fd::wstring_view name, const bool notify)
{
    LDR_DATA_TABLE_ENTRY* result;

    /* if (name.contains(':'))
    {
        std::vector<wchar_t> name_correct_buff;
        name_correct_buff.reserve(name.size());
        for (const auto chr : name)
            name_correct_buff.push_back(chr == '/' ? '\\' : chr);

        const fd::wstring_view name_correct(name_correct_buff.begin(), name_correct_buff.end());
        result = _Find_library([=](const fd::library_info info, void*, void*) {
            return info.path() == name_correct;
        });
    }
    else */
    {
        result = _Find_library([=](const fd::library_info info, void*, void*) {
            using namespace fd;
            return info.name() == name;
        });
    }

    if (notify)
        fd::invoke(on_library_found, name, result);
    return result;
}

static DECLSPEC_NOINLINE HMODULE _Get_current_module_handle()
{
    MEMORY_BASIC_INFORMATION info;
    constexpr SIZE_T info_size      = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] const auto len = VirtualQueryEx(GetCurrentProcess(), _Get_current_module_handle, &info, info_size);
    FD_ASSERT(len == info_size, "Wrong size");
    return static_cast<HMODULE>(info.AllocationBase);
}

LDR_DATA_TABLE_ENTRY* find_current_library(const bool notify)
{
    static const auto ret = [=] {
        const auto base_address = static_cast<void*>(_Get_current_module_handle());
        const auto ldr_entry    = _Find_library([=](void*, IMAGE_DOS_HEADER* const dos, void*) {
            return base_address == static_cast<void*>(dos);
        });

        if (notify)
        {
            using namespace fd;
            fd::invoke(
                on_library_found,
                [=] {
                    return fd::library_info(ldr_entry).name();
                },
                ldr_entry);
        }

        return ldr_entry;
    }();
    return ret;
}
