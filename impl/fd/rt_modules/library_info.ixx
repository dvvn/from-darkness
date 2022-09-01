module;

#include <windows.h>
#include <winternl.h>

#include <span>
#include <typeinfo>

export module fd.rt_modules:library_info;
export import fd.string;

using namespace fd;

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

#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

template <class T>
constexpr auto simple_type_name()
{
    return __FUNCSIG__;
}

struct correct_type_name : string_view
{
    constexpr correct_type_name(const string_view raw_name)
        : string_view(raw_name.data() + raw_name.find('<') + 1, raw_name.data() + raw_name.rfind('>')) // full name with 'class' or 'struct' prefix, namespace and templates
    {
    }
};

class rewrapped_namespaces
{
    struct simulate_info
    {
        size_t size      = 0;
        bool have_prefix = false;
    };

    template <size_t S>
    struct name_buff
    {
        char buff[S];
    };

    string_view prefix_;
    string_view name_;

  public:
    constexpr rewrapped_namespaces(const string_view corrected_name)
    {
        const auto space_pos = corrected_name.find(' ');
        if (space_pos == corrected_name.npos)
            name_ = corrected_name;
        else
        {
            prefix_ = corrected_name.substr(0, space_pos);
            name_   = corrected_name.substr(space_pos + 1);
        }
    }

    constexpr string_view name() const
    {
        return name_;
    }

    constexpr bool is_class() const
    {
        return prefix_ == "class";
    }

    constexpr bool is_struct() const
    {
        return prefix_ == "struct";
    }

    constexpr size_t calc_size() const
    {
        const auto namespaces_chars = std::count(name_.begin(), name_.end(), ':');
        const size_t prefix_size    = 0 /*prefix_.size()*/;
        const size_t postfix_size   = 0 /*2*/;
        return prefix_size + name_.size() - namespaces_chars / 2 + postfix_size;
    }

    template <size_t S>
    constexpr name_buff<S> get() const
    {
        name_buff<S> ret{};

        auto itr       = name_.data();
        // copy prefix if exist
        /* if (auto space_pos = name_.find(' '); space_pos != name_.npos)
        {
            ++space_pos;
            std::copy_n(itr, space_pos, ret.buff);
            itr += space_pos;
        } */
        char* buff_end = ret.buff + S;
        // create postfix
        /* *--buff_end = '@';
         *--buff_end = '@'; */
        const auto end = name_.data() + name_.size();
        for (;;)
        {
            const auto word_end  = std::find(itr, end, ':');
            const auto word_size = std::distance(itr, word_end);
            std::copy_n(itr, word_size, buff_end -= word_size);
            if (word_end == end)
                break;
            *--buff_end = '@';
            itr += word_size + 2;
        }

        return ret;
    }
};

struct library_info
{
    using pointer   = const LDR_DATA_TABLE_ENTRY*;
    using reference = const LDR_DATA_TABLE_ENTRY&;

  private:
    pointer entry_;

  public:
    static bool exists(const wstring_view name);

    library_info(pointer const entry);
    library_info(const wstring_view name, const bool notify = true);
    library_info(IMAGE_DOS_HEADER* const base_address, const bool notify = true);

    bool is_root() const;
    bool unload() const;

    pointer get() const;
    pointer operator->() const;
    reference operator*() const;

    wstring_view path() const;
    wstring_view name() const;

    void log_class_info(const string_view raw_name, const void* addr) const;

    template <class T>
    void log_class_info(const T* addr) const
    {
        constexpr correct_type_name name(simple_type_name<T>());
        log_class_info(name, addr);
    }

    void* find_export(const string_view name, const bool notify = true) const;
    IMAGE_SECTION_HEADER* find_section(const string_view name, const bool notify = true) const;
    void* find_signature(const string_view sig, const bool notify = true) const;

  private:
    void* find_vtable_class(const string_view name, const bool notify) const;
    void* find_vtable_struct(const string_view name, const bool notify) const;
    void* find_vtable_unknown(const string_view name, const bool notify) const;

  public:
    void* find_vtable(const string_view name, const bool notify = true) const;
    void* find_vtable(const std::type_info& info, const bool notify = true) const;

    template <class T>
    T* find_vtable(const bool notify = true) const
    {
        constexpr correct_type_name name(simple_type_name<T>());
        void* ptr;
        if constexpr (name.contains('<')) // templates currently unsupported
        {
            ptr = find_vtable(typeid(T), notify);
        }
        else if constexpr (name.contains(':'))
        {
            constexpr rewrapped_namespaces corr_name(name);
            constexpr auto size = corr_name.calc_size();
            constexpr auto ret  = corr_name.get<size>();
            constexpr string_view result(ret.buff, size);
            // const string_view test = typeid(T).raw_name();
            if constexpr (corr_name.is_class())
                ptr = find_vtable_class(result, notify);
            else if constexpr (corr_name.is_struct())
                ptr = find_vtable_struct(result, notify);
            else
            {
                // ptr = find_vtable_unknown(result, notify);
                static_assert(std::_Always_false<T>, "Wrong prefix!");
            }
        }
        else
        {
            ptr = find_vtable(name, notify);
        }
        return static_cast<T*>(ptr);
    }

    void* find_csgo_interface(const string_view name, const bool notify = true) const;
    void* find_csgo_interface(const void* create_interface_fn, const string_view name, const bool notify = true) const;
};

export namespace fd
{
    using ::dos_nt;
    using ::library_info;
} // namespace fd
