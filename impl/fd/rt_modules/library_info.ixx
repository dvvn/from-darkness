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

template <class T>
constexpr auto simple_type_name()
{
    return __FUNCSIG__;
}

#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

struct library_info
{
    using pointer   = const LDR_DATA_TABLE_ENTRY*;
    using reference = const LDR_DATA_TABLE_ENTRY&;

  private:
    pointer entry_;

  public:
    library_info(pointer const entry);
    library_info(const fd::wstring_view name, const bool notify = true);
    library_info(IMAGE_DOS_HEADER* const base_address, const bool notify = true);

    pointer get() const;
    pointer operator->() const;
    reference operator*() const;

    fd::wstring_view path() const;
    fd::wstring_view name() const;

    void log_class_info(const fd::string_view raw_name, const void* addr) const;

    void* find_export(const fd::string_view name, const bool notify = true) const;
    IMAGE_SECTION_HEADER* find_section(const fd::string_view name, const bool notify = true) const;
    void* find_signature(const fd::string_view sig, const bool notify = true) const;

    [[deprecated]] void* find_vtable_class(const fd::string_view name, const bool notify = true) const;
    [[deprecated]] void* find_vtable_struct(const fd::string_view name, const bool notify = true) const;
    void* find_vtable(const fd::string_view name, const bool notify = true) const;
    void* find_vtable(decltype(typeid(int)) info, const bool notify = true) const;

    template <class T>
    T* find_vtable(const bool notify = true) const
    {
        constexpr fd::string_view raw_name = simple_type_name<T>();
        // full name with 'class' or 'struct' prefix, namespace and templates
        constexpr fd::string_view name(raw_name.data() + raw_name.find('<') + 1, raw_name.data() + raw_name.rfind('>'));

        void* ptr;
        if constexpr (name.contains('>') || name.contains(':')) // namespaces and templates currently unsupported
            ptr = find_vtable(typeid(T), notify);
        else
            ptr = find_vtable(name, notify);
        return static_cast<T*>(ptr);
    }

    void* find_csgo_interface(const fd::string_view name, const bool notify = true) const;
    void* find_csgo_interface(const void* create_interface_fn, const fd::string_view name, const bool notify = true) const;
};

export namespace fd
{
    using ::dos_nt;
    using ::library_info;
} // namespace fd
