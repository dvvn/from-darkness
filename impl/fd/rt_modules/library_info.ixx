module;

#include <fd/rt_modules/winapi.h>

#include <span>

export module fd.rt_modules:library_info;
export import fd.string;

struct library_info;

class dos_nt
{
    void _Construct(const LDR_DATA_TABLE_ENTRY* const ldr_entry);

  public:
    // base address
    IMAGE_DOS_HEADER* dos;
    IMAGE_NT_HEADERS* nt;

    dos_nt(const LDR_DATA_TABLE_ENTRY* const ldr_entry);
    explicit dos_nt(const library_info info);

    std::span<uint8_t> read() const;
    std::span<IMAGE_SECTION_HEADER> sections() const;

    template <typename T = uint8_t, typename Q>
    T* map(Q obj) const
    {
        const auto dos_addr = reinterpret_cast<uintptr_t>(dos);
        uintptr_t offset;
        if constexpr (std::is_pointer_v<Q>)
            offset = reinterpret_cast<uintptr_t>(obj);
        else
            offset = static_cast<uintptr_t>(obj);
        return reinterpret_cast<T*>(dos_addr + offset);
    }
};

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
    using ::dos_nt;
    using ::library_info;
}
