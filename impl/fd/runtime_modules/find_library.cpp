module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <functional>
#include <string_view>

module fd.rt_modules:find_library;
import :library_info;
import :helpers;
import fd.chars_cache;
import fd.logger;

static void on_library_found(std::wstring_view, LDR_DATA_TABLE_ENTRY*)
{
    std::invoke(fd::logger, "Library found! (WIP)");
}

template <typename Fn>
class partial_invoke
{
    template <class Tpl, size_t... I>
    static constexpr auto _Get_valid_seq(const std::index_sequence<I...> seq)
    {
        if constexpr (std::invocable<Fn, std::tuple_element_t<I, Tpl>...>)
            return seq;
        else if constexpr (seq.size() > 0)
            return _Get_valid_seq<Tpl>(std::make_index_sequence<seq.size() - 1>());
        else
            return std::false_type();
    }

    template <class Tpl, size_t I1 = -1, size_t... I>
    static constexpr auto _Get_valid_seq_reversed(const std::index_sequence<I1, I...> seq = {})
    {
        if constexpr (I1 == -1)
            return std::false_type();
        else if constexpr (std::invocable<Fn, std::tuple_element_t<I1, Tpl>, std::tuple_element_t<I, Tpl>...>)
            return seq;
        else
            return _Get_valid_seq_reversed<Tpl, I...>();
    }

    template <class Tpl, size_t... I>
    auto apply(Tpl& tpl, const std::index_sequence<I...>) const
    {
        return std::invoke(fn_, std::get<I>(tpl)...);
    }

    template <class Tpl, class SeqFwd, class SeqBack>
    auto apply(Tpl& tpl, const SeqFwd seq_fwd, const SeqBack seq_back) const
    {
        constexpr auto fwd_ok = !std::same_as<SeqFwd, std::false_type>;
        constexpr auto bk_ok  = !std::same_as<SeqBack, std::false_type>;

        static_assert((fwd_ok || bk_ok) && !(fwd_ok && bk_ok), "Unable to choice the sequence!");

        if constexpr (fwd_ok)
            return this->apply<Tpl>(tpl, seq_fwd);
        else
            return this->apply<Tpl>(tpl, seq_back);
    }

  public:
    template <typename FnRef>
    partial_invoke(FnRef&& fn)
        : fn_(std::forward<FnRef>(fn))
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const
    {
        auto tpl    = std::forward_as_tuple(std::forward<Args>(args)...);
        using tpl_t = decltype(tpl);

        constexpr std::make_index_sequence<sizeof...(Args)> seq_def;
        constexpr auto seq          = _Get_valid_seq<tpl_t>(seq_def);
        constexpr auto seq_reversed = _Get_valid_seq_reversed<tpl_t>(seq_def);

        return this->apply(tpl, seq, seq_reversed);
    }

  private:
    Fn fn_;
};

template <typename Fn>
partial_invoke(Fn&&) -> partial_invoke<std::remove_cvref_t<Fn>>;

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
    const partial_invoke invoker = std::move(comparer);
    const auto ldr               = _Get_ldr();
    // get module linked list.
    const auto list              = &ldr->InMemoryOrderModuleList;
    // iterate linked list.
    for (auto it = list->Flink; it != list; it = it->Flink)
    {
        // get current entry.
        const auto ldr_entry = CONTAINING_RECORD(it, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (!ldr_entry)
            continue;

        const auto [dos, nt] = fd::dos_nt(ldr_entry);
        if (!invoker(ldr_entry, nt, dos))
            continue;

        return ldr_entry;
    }

    return nullptr;
}

#ifndef __cpp_lib_string_contains
#define contains(_X_) find(_X_) != std::wstring_view::npos
#endif

LDR_DATA_TABLE_ENTRY* find_library(const std::wstring_view name, const bool notify)
{
    LDR_DATA_TABLE_ENTRY* result;

    /* if (name.contains(':'))
    {
        std::vector<wchar_t> name_correct_buff;
        name_correct_buff.reserve(name.size());
        for (const auto chr : name)
            name_correct_buff.push_back(chr == '/' ? '\\' : chr);

        const std::wstring_view name_correct(name_correct_buff.begin(), name_correct_buff.end());
        result = _Find_library([=](const fd::library_info info) {
            return info.path() == name_correct;
        });
    }
    else */
    {
        result = _Find_library([=](const fd::library_info info) {
            return info.name() == name;
        });
    }

    if (notify)
        std::invoke(on_library_found, name, result);
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
        const auto ldr_entry    = _Find_library([=](IMAGE_DOS_HEADER* const dos) {
            return base_address == static_cast<void*>(dos);
        });

        if (notify)
        {
            using namespace fd;
            std::invoke(on_library_found, L"current"_cch, ldr_entry);
        }

        return ldr_entry;
    }();
    return ret;
}
