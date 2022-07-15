module;

#include <windows.h>
#include <winternl.h>

export module fd.rt_modules:library_info;
export import fd.string;

struct library_info
{
    using pointer   = const LDR_DATA_TABLE_ENTRY*;
    using reference = const LDR_DATA_TABLE_ENTRY&;

    library_info(pointer const entry);

    pointer operator->() const;
    reference operator*() const;

    fd::wstring_view path() const;
    fd::wstring_view name() const;

    void log(const fd::string_view object_type, const fd::string_view object, const void* addr) const;

  private:
    pointer entry_;
};

export namespace fd
{
    using ::library_info;
}
