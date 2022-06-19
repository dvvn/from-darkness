module;

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:library_info;

struct library_info
{
    using pointer   = const LDR_DATA_TABLE_ENTRY*;
    using reference = const LDR_DATA_TABLE_ENTRY&;

    library_info(pointer const entry);

    pointer   operator->() const;
    reference operator*() const;

    std::wstring_view path() const;
    std::wstring_view name() const;

  private:
    pointer entry_;
};

export namespace fd
{
    using ::library_info;
}
