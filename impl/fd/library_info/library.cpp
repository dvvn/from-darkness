#include "library.h"
#include "library_name.h"

#include <windows.h>
#include <winternl.h>

#include <cassert>

#if !defined(_delayimp_h) && _MSC_VER >= 1300
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace fd
{
template <typename T, bool Deref>
class win_list_view_iterator
{
    using entry_pointer = LIST_ENTRY *;

    entry_pointer current_;

  public:
    win_list_view_iterator(entry_pointer ptr = nullptr)
        : current_(ptr)
    {
    }

    win_list_view_iterator &operator++()
    {
        current_ = current_->Flink;
        return *this;
    }

    win_list_view_iterator operator++(int)
    {
        auto c   = current_;
        current_ = current_->Flink;
        return c;
    }

    win_list_view_iterator &operator--()
    {
        current_ = current_->Blink;
        return *this;
    }

    win_list_view_iterator operator--(int)
    {
        auto c   = current_;
        current_ = current_->Blink;
        return c;
    }

    T &operator*() const requires(Deref)
    {
        return *CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
    }

    T *operator*() const requires(!Deref)
    {
        return CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
    }

    T *operator->() const
    {
        return CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
    }

    bool operator==(T const *other) const
    {
        return CONTAINING_RECORD(current_, T, InMemoryOrderLinks) == other;
    }

    T *get() const
    {
        return CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
    }

    bool operator==(win_list_view_iterator const &) const = default;
};

template <typename T, bool Deref = true>
struct win_list_view
{
    using pointer   = LIST_ENTRY *;
    using reference = LIST_ENTRY &;

    using iterator = win_list_view_iterator<T, Deref>;

  private:
    pointer root_;

  public:
    win_list_view(LIST_ENTRY *root = nullptr)
    {
        if (root)
        {
            root_ = root;
            return;
        }

#if defined(_M_X64) || defined(__x86_64__)
        auto mem = NtCurrentTeb();
        auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
        auto mem = reinterpret_cast<PEB *>(__readfsdword(0x30));
        auto ldr = mem->Ldr;
#endif
        // get module linked list.
        root_ = &ldr->InMemoryOrderModuleList;
    }

    iterator begin() const
    {
        return root_->Flink;
    }

    iterator end() const
    {
        return root_;
    }
};
} // namespace fd

template <typename T, bool Deref>
struct std::iterator_traits<fd::win_list_view_iterator<T, Deref>>
{
    // using iterator_concept  = bidirectional_iterator_tag;
    using iterator_category = bidirectional_iterator_tag;
    using value_type        = std::conditional_t<Deref, T, T *>;
    using difference_type   = ptrdiff_t;
    using pointer           = T *;
    using reference         = std::conditional_t<Deref, T &, T *>;
};

#ifdef __cpp_lib_ranges
template <typename T, bool Deref>
constexpr bool std::ranges::enable_borrowed_range<fd::win_list_view<T, Deref>> = true;
#endif

namespace fd
{
template <typename T, bool Deref>
static bool operator==(T const *other, typename win_list_view<T, Deref>::iterator itr)
{
    return itr.operator==(other);
}

using ldr_tables_view     = win_list_view<LDR_DATA_TABLE_ENTRY>;
using ldr_tables_view_ptr = win_list_view<LDR_DATA_TABLE_ENTRY, false>;

LDR_DATA_TABLE_ENTRY *find_library(void *base_address)
{
    for (auto *e : ldr_tables_view_ptr())
    {
        if (base_address == e->DllBase)
            return e;
    }
    return nullptr;
}

LDR_DATA_TABLE_ENTRY *find_library(wchar_t const *name, size_t length)
{
    auto test_name = std::basic_string_view(name, length);

    for (auto *e : ldr_tables_view_ptr())
    {
        if (!e->FullDllName.Buffer)
            continue;
#if 0
        auto lib_name = library_name(library_path(e));
        if(lib_name != test_name)
            continue;
#else
        auto lib_path = library_path(e);
        if (!lib_path.ends_with(test_name))
            continue;
        if (*(lib_path.rbegin() + test_name.size()) != '\\')
            continue;
#endif
        return e;
    }

    return nullptr;
}

LDR_DATA_TABLE_ENTRY *const this_library = find_library(
#ifdef _MSC_VER
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