// ReSharper disable CppClangTidyClangDiagnosticMicrosoftCast
#include <fd/dll_notification.h>
#include <fd/library_info.h>
#include <fd/mem_scanner.h>

#include <spdlog/spdlog.h>

#include <boost/range/adaptors.hpp>
#include <boost/type_index.hpp>

#include <array>
#include <semaphore>
#include <span>

template <typename T>
static bool operator==(std::basic_string_view<T> left, fmt::basic_string_view<T> right)
{
    auto size = left.size();
    return size == right.size() && std::char_traits<T>::compare(left.data(), right.data(), size) == 0;
}

template <typename T>
static bool operator==(fmt::basic_string_view<T> left, std::basic_string_view<T> right)
{
    auto size = left.size();
    return size == right.size() && std::char_traits<T>::compare(left.data(), right.data(), size) == 0;
}

template <typename ContextC, typename StrC, bool Same = std::same_as<ContextC, StrC>>
class chars_mixer;

template <typename ContextC, typename StrC>
class chars_mixer<ContextC, StrC, true>
{
    fmt::basic_string_view<StrC> cached_;

  public:
    chars_mixer(std::basic_string_view<StrC> str = {})
        : cached_(str)
    {
    }

    auto native() const
    {
        return cached_;
    }
};

template <typename ContextC, typename StrC>
class chars_mixer<ContextC, StrC, false>
{
    using context_str = fmt::basic_string_view<ContextC>;
    using current_str = fmt::basic_string_view<StrC>;

    using current_str_in = std::basic_string_view<StrC>;

    current_str cached_;

  public:
    static_assert(sizeof(ContextC) >= sizeof(StrC));

    chars_mixer() = default;

    chars_mixer(StrC const* ptr)
        : cached_(ptr)
    {
    }

    chars_mixer(current_str_in str)
        : cached_(str)
    {
    }

    operator current_str() const
    {
        return cached_;
    }

    class buffer
    {
        fmt::basic_memory_buffer<ContextC> buffer_;

      public:
        buffer(current_str const str)
        {
            buffer_.append(str);
        }

        operator context_str() const
        {
            return { buffer_.data(), buffer_.size() };
        }
    };

    buffer native() const
    {
        return cached_;
    }
};

template <typename ContextC, typename StrC>
struct fmt::formatter<chars_mixer<ContextC, StrC>, ContextC> : formatter<basic_string_view<ContextC>, ContextC>
{
    auto format(chars_mixer<ContextC, StrC> const& mixer, auto& ctx) const
    {
        return formatter<basic_string_view<ContextC>, ContextC>::format(mixer.native(), ctx);
    }
};

using lazy_wstring = chars_mixer<wchar_t, char>;

enum class valve_ifc_reg_iterator_mode : uint8_t
{
    normal,
    compare,
    const_compare
};

class valve_ifc_reg
{
    template <valve_ifc_reg_iterator_mode>
    friend class valve_ifc_reg_iterator;

    void* (*createFn_)();
    char const*    name_;
    valve_ifc_reg* next_;

  public:
#if 0
    class range
    {
        valve_ifc_reg* root_;

      public:
        range(void const* createInterfaceFn)
        {
            auto relativeFn   = reinterpret_cast<uintptr_t>(createInterfaceFn) + 0x5;
            auto displacement = *reinterpret_cast<int32_t*>(relativeFn);
            auto jmp          = relativeFn + sizeof(int32_t) + displacement;

            root_ = **reinterpret_cast<valve_ifc_reg***>(jmp + 0x6);
        }

        range(valve_ifc_reg* current)
            : root_(current)
        {
        }

        iterator begin() const
        {
            return root_;
        }

        iterator end() const
        {
            (void)this;
            return nullptr;
        }
    };
#endif

    valve_ifc_reg()                     = delete;
    valve_ifc_reg(valve_ifc_reg const&) = delete;

    void* operator()() const
    {
        return createFn_();
    }

    char const* name() const
    {
        return name_;
    }

    valve_ifc_reg const* operator+(size_t offset) const
    {
        switch (offset)
        {
        case 0:
            return this;
        case 1:
            return next_;
        default:
            for (auto src = this;;)
            {
                src = src->next_;
                if (--offset == 0)
                    return src;
            }
        }
    }
};

enum class valve_ifc_reg_cmp_result : uint8_t
{
    error,
    full,
    partial,
    unset
};

template <valve_ifc_reg_iterator_mode Mode>
class valve_ifc_reg_iterator
{
    using mode = valve_ifc_reg_iterator_mode;

    using cmp_result = valve_ifc_reg_cmp_result;
    using cmp_type   = std::conditional_t<Mode == mode::normal, std::false_type, cmp_result>;

    valve_ifc_reg*                 current_;
    [[no_unique_address]] cmp_type compared_;

    template <mode>
    friend class valve_ifc_reg_iterator;

    valve_ifc_reg_iterator(valve_ifc_reg* ptr, cmp_type cmp)
        : current_(ptr)
        , compared_(cmp)
    {
    }

  public:
    valve_ifc_reg_iterator(valve_ifc_reg_iterator const&) = default;

    template <mode M>
    valve_ifc_reg_iterator(valve_ifc_reg_iterator<M> other)
        : current_(other.current_)
        , compared_(other.compared_)
    {
        if constexpr (Mode == mode::const_compare)
            assert(compared_ != cmp_result::error);
    }

    valve_ifc_reg_iterator(valve_ifc_reg* reg)
        : current_(reg)
        , compared_(cmp_result::unset)
    {
    }

    valve_ifc_reg_iterator& operator++()
    {
        current_ = current_->next_;
        return *this;
    }

    valve_ifc_reg_iterator operator++(int)
    {
        auto tmp = current_;
        current_ = current_->next_;
        return tmp;
    }

    valve_ifc_reg_iterator& operator*() requires(Mode != mode::normal)
    {
        return *this;
    }

    void set(cmp_result status) requires(Mode != mode::normal)
    {
        compared_ = status;
    }

    valve_ifc_reg const& operator*() const
    {
        return *this->current_;
    }

    valve_ifc_reg* operator->() const
    {
        return current_;
    }

    template <mode M>
    bool operator==(valve_ifc_reg_iterator<M> const other) const
    {
        return current_ == other.current_;
    }

    bool operator==(std::nullptr_t) const
    {
        return !current_;
    }

    valve_ifc_reg_iterator operator+(size_t i) const
    {
        assert(i == 1);
        return { current_->next_, compared_ };
    }

    cmp_result status() const
    {
        return compared_;
    }
};

template <valve_ifc_reg_iterator_mode Mode>
struct std::iterator_traits<valve_ifc_reg_iterator<Mode>>
{
    using type       = std::forward_iterator_tag;
    using value_type = valve_ifc_reg;
};

template <typename C>
bool operator==(valve_ifc_reg_iterator<valve_ifc_reg_iterator_mode::compare>& it, std::basic_string_view<C> ifcName)
{
    auto& curr     = *std::as_const(it);
    auto  currName = std::string_view(curr.name());

    auto cmp = valve_ifc_reg_cmp_result::error;
    if (currName.starts_with(ifcName))
    {
        if (currName.size() == ifcName.size())
            cmp = valve_ifc_reg_cmp_result::full;
        else if (std::isdigit(currName[ifcName.size()]))
            cmp = valve_ifc_reg_cmp_result::partial;
    }
    it.set(cmp);
    return cmp != valve_ifc_reg_cmp_result::error;
}

template <typename C>
bool operator==(
    valve_ifc_reg_iterator<valve_ifc_reg_iterator_mode::const_compare> const& it,
    std::basic_string_view<C>                                                 ifcName)
{
    switch (it.status())
    {
    case valve_ifc_reg_cmp_result::full:
    {
        return it->name() == ifcName;
    }
    case valve_ifc_reg_cmp_result::partial:
    {
        auto currName = std::string_view(it->name());
        if (currName.size() <= ifcName.size())
            return false;
        if (!std::isdigit(currName[ifcName.size()]))
            return false;
        return currName.starts_with(ifcName);
    }
    default:
    {
        assert(0 && "Wrong state");
        return false;
    }
    }
}

static valve_ifc_reg* _root_interface(void const* createInterfaceFn)
{
    auto relativeFn   = reinterpret_cast<uintptr_t>(createInterfaceFn) + 0x5;
    auto displacement = *reinterpret_cast<int32_t*>(relativeFn);
    auto jmp          = relativeFn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<valve_ifc_reg***>(jmp + 0x6);
}

template <typename T>
struct fmt::formatter<valve_ifc_reg, T> : formatter<basic_string_view<T>, T>
{
    auto format(valve_ifc_reg const& reg, auto& ctx) const
    {
        auto tmp = chars_mixer<T, char>(reg.name());
        return formatter<basic_string_view<T>, T>::format(tmp.native(), ctx);
    }
};

//---------

// struct dos_entry : fd::hidden_ptr
//{
//     dos_entry(IMAGE_DOS_HEADER const* dos)
//         : hidden_ptr(dos)
//     {
//     }
// };

struct dos_header : fd::hidden_ptr
{
    dos_header(IMAGE_DOS_HEADER const* dos)
        : hidden_ptr(dos)
    {
    }

    IMAGE_DOS_HEADER const* operator->() const
    {
        return *this;
    }

    IMAGE_DOS_HEADER const& operator*() const
    {
        return *operator->();
    }
};

static IMAGE_DOS_HEADER* _get_dos(LDR_DATA_TABLE_ENTRY* entry)
{
    auto dos = static_cast<IMAGE_DOS_HEADER*>(entry->DllBase);
    // check for invalid DOS / DOS signature.
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    return dos;
}

static IMAGE_NT_HEADERS* _get_nt(dos_header dos)
{
    IMAGE_NT_HEADERS* nt = dos + dos->e_lfanew;
    // check for invalid NT / NT signature.
    assert(nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
    return nt;
}

static IMAGE_NT_HEADERS* _get_nt(LDR_DATA_TABLE_ENTRY* entry)
{
    return _get_nt(_get_dos(entry));
}

static std::span<IMAGE_SECTION_HEADER> _sections(IMAGE_NT_HEADERS const* nt)
{
    assert(nt != nullptr);
    return { IMAGE_FIRST_SECTION(nt), static_cast<size_t>(nt->FileHeader.NumberOfSections) };
}

static std::span<IMAGE_SECTION_HEADER> _sections(LDR_DATA_TABLE_ENTRY* entry)
{
    return _sections(_get_nt(entry));
}

static std::span<uint8_t> _memory(LDR_DATA_TABLE_ENTRY* entry, IMAGE_NT_HEADERS const* nt = nullptr)
{
    if (!nt)
        nt = _get_nt(entry);
    //(void)nt->OptionalHeader.BaseOfCode;
    //(void)nt->OptionalHeader.BaseOfData;
    //(void)nt->OptionalHeader.ImageBase; //same as entry->DllBase
    return { static_cast<uint8_t*>(entry->DllBase), nt->OptionalHeader.SizeOfImage };
}

static IMAGE_SECTION_HEADER* _find_section(std::span<IMAGE_SECTION_HEADER> rng, std::string_view name)
{
    for (auto& h : rng)
    {
        if (reinterpret_cast<char const*>(h.Name) == name)
            return &h;
    }
    return nullptr;
}

static IMAGE_SECTION_HEADER* _find_section(LDR_DATA_TABLE_ENTRY* entry, std::string_view name)
{
    return _find_section(_sections(entry), name);
}

class dll_exports
{
    dos_header dos_;

    uint32_t* names_;
    uint32_t* funcs_;
    uint16_t* ords_;

    union
    {
        IMAGE_EXPORT_DIRECTORY* exportDirDir_;
        uint8_t*                virtualAddrStart_;
    };

    uint8_t* virtualAddrEnd_;

    using src_ptr = dll_exports const*;

  public:
    dll_exports(IMAGE_DOS_HEADER* dos, IMAGE_NT_HEADERS* nt = nullptr)
        : dos_(dos)
    {
        if (!nt)
            nt = _get_nt(dos_);

        auto const& dataDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        // get export export_dir.
        exportDirDir_   = dos_ + dataDir.VirtualAddress;
        virtualAddrEnd_ = virtualAddrStart_ + dataDir.Size;

        names_ = dos_ + exportDirDir_->AddressOfNames;
        funcs_ = dos_ + exportDirDir_->AddressOfFunctions;
        ords_  = dos_ + exportDirDir_->AddressOfNameOrdinals;
    }

    dll_exports(_LDR_DATA_TABLE_ENTRY* entry)
        : dll_exports(_get_dos(entry))
    {
    }

    char const* name(DWORD offset) const
    {
        return dos_ + names_[offset];
    }

    void* function(DWORD offset) const
    {
        void* tmp = dos_ + funcs_[ords_[offset]];
        if (tmp < virtualAddrStart_ || tmp >= virtualAddrEnd_)
            return tmp;

        // todo:resolve fwd export
        assert(0 && "Forwarded export detected");
        return nullptr;

#if 0
		// get forwarder std::string.
		std::string_view fwd_str = export_ptr.get<const char*>( );

		// forwarders have a period as the delimiter.
		auto delim = fwd_str.find_last_of('.');
		if(delim == fwd_str.npos)
			continue;

		using namespace string_view_literals;
		// get forwarder mod name.
		const info_string::fixed_type fwd_module_str = nstd::append<std::wstring>(fwd_str.substr(0, delim), L".dll"sv);

		// get real export ptr ( recursively ).
		auto target_module = std::ranges::find_if(*all_modules, [&](const info& i)
		{
			return i.name == fwd_module_str;
		});
		if(target_module == all_modules->end( ))
			continue;

		// get forwarder export name.
		auto fwd_export_str = fwd_str.substr(delim + 1);

		try
		{
			auto& exports = target_module->exports( );
			auto fwd_export_ptr = exports.at(fwd_export_str);

			this->emplace(export_name, fwd_export_ptr);
		}
		catch(std::exception)
		{
		}
#endif
    }

    class wrapper
    {
        src_ptr src_;
        DWORD   offset_;

      public:
        wrapper(src_ptr src, DWORD offset)
            : src_(src)
            , offset_(offset)
        {
        }

        char const* name() const
        {
            return src_->name(offset_);
        }

        void* function() const
        {
            return src_->function(offset_);
        }
    };

    class iterator
    {
        src_ptr src_;
        DWORD   offset_;

      public:
        iterator(src_ptr src, DWORD offset)
            : src_(src)
            , offset_(offset)
        {
        }

        iterator& operator++()
        {
            ++offset_;
            return *this;
        }

        iterator& operator--()
        {
            --offset_;
            return *this;
        }

        wrapper operator*() const
        {
            return { src_, offset_ };
        }

        bool operator==(iterator const& other) const
        {
            if (offset_ == other.offset_)
            {
                assert(this->src_ == other.src_);
                return true;
            }
            return false;
        }
    };

    iterator begin() const
    {
        return { this, 0 };
    }

    iterator end() const
    {
        return { this, std::min(exportDirDir_->NumberOfNames, exportDirDir_->NumberOfFunctions) };
    }
};

static void* _find_export(dll_exports const& exports, std::string_view name)
{
    for (auto val : exports)
    {
        if (val.name() == name)
            return val.function();
    }
    return nullptr;
}

template <typename T, bool Deref = true>
class win_list_view
{
    using pointer   = LIST_ENTRY*;
    using reference = LIST_ENTRY&;

    pointer root_;

  public:
    class iterator
    {
        pointer current_;

      public:
        iterator(pointer ptr = nullptr)
            : current_(ptr)
        {
        }

        iterator& operator++()
        {
            current_ = current_->Flink;
            return *this;
        }

        iterator operator++(int)
        {
            auto c   = current_;
            current_ = current_->Flink;
            return c;
        }

        iterator& operator--()
        {
            current_ = current_->Blink;
            return *this;
        }

        iterator operator--(int)
        {
            auto c   = current_;
            current_ = current_->Blink;
            return c;
        }

        T& operator*() const requires(Deref)
        {
            return *CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
        }

        T* operator*() const requires(!Deref)
        {
            return CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
        }

        T* operator->() const
        {
            return CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
        }

        bool operator==(T const* other) const
        {
            return CONTAINING_RECORD(current_, T, InMemoryOrderLinks) == other;
        }

        bool operator==(iterator const&) const = default;
    };

    win_list_view(LIST_ENTRY* root = nullptr)
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
        auto mem = reinterpret_cast<PEB*>(__readfsdword(0x30));
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

template <typename T, bool Deref>
static bool operator==(T const* other, typename win_list_view<T, Deref>::iterator itr)
{
    return itr.operator==(other);
}

using ldr_tables_view     = win_list_view<LDR_DATA_TABLE_ENTRY>;
using ldr_tables_view_ptr = win_list_view<LDR_DATA_TABLE_ENTRY, false>;

static void _to_string_view(LDR_DATA_TABLE_ENTRY* entry) = delete;

static auto _to_string_view(UNICODE_STRING const& ustr)
{
    std::wstring_view ret;
    if (ustr.Buffer)
        std::construct_at(&ret, ustr.Buffer, ustr.Length / sizeof(WCHAR));
    return ret;
}

static auto _to_string(UNICODE_STRING const& ustr)
{
    return std::wstring(_to_string_view(ustr));
}

static auto _library_info_path(LDR_DATA_TABLE_ENTRY* entry)
{
    return _to_string_view(entry->FullDllName);
}

class library_info_name
{
    LDR_DATA_TABLE_ENTRY* entry;

  public:
    library_info_name(LDR_DATA_TABLE_ENTRY* entry)
        : entry(entry)
    {
    }

    operator std::wstring_view() const
    {
        auto fullPath = _library_info_path(entry);
#if 0
    return std::wstring_view(file_view(fullPath).name());
#else
        auto nameStart = fullPath.rfind('\\');
        assert(nameStart != fullPath.npos);
        return fullPath.substr(nameStart + 1);
#endif
    }
};

template <typename T>
struct fmt::formatter<library_info_name, T> : formatter<basic_string_view<T>, T>
{
    auto format(library_info_name name, auto& ctx) const
    {
        auto tmp = chars_mixer<T, wchar_t>(name);
        return formatter<basic_string_view<T>, T>::format(tmp.native(), ctx);
    }
};

static void _log_found_entry(std::wstring_view name, /*LDR_DATA_TABLE_ENTRY*/ void const* entry)
{
    if (entry)
        spdlog::info(L"{} -> found! ({:p})", name, entry);
    else
        spdlog::warn(L"{} -> NOT found!", name);
}

static void _log_found_entry_or_wait(std::wstring_view name, /*LDR_DATA_TABLE_ENTRY*/ void const* entry)
{
    if (entry)
        spdlog::info(L"{} -> found! ({:p})", name, entry);
    else
        spdlog::warn(L"{} -> NOT found! Waiting...", name);
}

static void _log_found_entry(/*IMAGE_DOS_HEADER*/ void const* baseAddress, LDR_DATA_TABLE_ENTRY* entry)
{
    if (entry)
        spdlog::info(L"{:p} ({}) -> found! ({:p})", baseAddress, library_info_name(entry), fmt::ptr(entry));
    else
        spdlog::warn(L"{:p} -> NOT found!", baseAddress);
}

// template <>
// struct fmt::formatter<lazy_wstring, wchar_t> : fmt::formatter<std::wstring, wchar_t>
//{
//     template <typename FormatCtx>
//     auto format(lazy_wstring str, FormatCtx& ctx) const
//     {
//         return formatter<std::wstring, wchar_t>::format(std::wstring(str), ctx);
//     }
// };

static auto _log_found_csgo_interface(
    library_info_name        entry,
    valve_ifc_reg_cmp_result type,
    lazy_wstring             name,
    valve_ifc_reg*           reg)
{
    switch (type)
    {
    case valve_ifc_reg_cmp_result::error:
        spdlog::warn(L"{} -> csgo interface '{}' NOT found!", entry, name);
        break;
    case valve_ifc_reg_cmp_result::full:
        spdlog::info(L"{} -> csgo interface '{}' found! ({:p})", entry, name, fmt::ptr(reg));
        break;
    case valve_ifc_reg_cmp_result::partial:
        spdlog::info(L"{} -> csgo interface '{}' ({}) found! ({:p})", entry, name, *reg, fmt::ptr(reg));
        break;
    default:
        assert(0 && "Wrong state detected");
        break;
    }
}

static auto _log_found_object(
    library_info_name entry,
    std::wstring_view objectType,
    lazy_wstring      object,
    void const*       addr)
{
    if (addr)
        spdlog::info(L"{} -> {} '{}' found! ({:p})", entry, objectType, object, addr);
    else
        spdlog::warn(L"{} -> {} '{}' NOT found!", entry, objectType, object);
}

// static void _log_address_found(library_info_name entry, _object_type_fancy rawName, void const* addr)
//{
//     if (addr)
//         spdlog::info(L"{} -> {} found! ({:p})", entry, rawName, addr);
//     else
//         spdlog::warn(L"{} -> {} NOT found!", entry, rawName);
// }

static std::string_view _object_type(char const* rawName)
{
    assert(memcmp(rawName, ".?A", 3) == 0);
    auto prefix = *(rawName + 3);
    if (prefix == 'V')
        return "class";
    if (prefix == 'U')
        return "struct";
    return static_cast<char const*>(nullptr);
}

enum class obj_type : uint8_t
{
    // ReSharper disable CppInconsistentNaming
    UNKNOWN,
    CLASS,
    STRUCT
    // ReSharper restore CppInconsistentNaming
};

template <typename T>
struct fmt::formatter<obj_type, T> : formatter<basic_string_view<T>, T>
{
    auto format(obj_type type, auto& ctx) const
    {
        chars_mixer<T, char> buff;

        switch (type)
        {
        case obj_type::CLASS:
            buff = "class";
            break;
        case obj_type::STRUCT:
            buff = "struct";
            break;
        default:;
            throw format_error("Unknwon object type");
        }

        return formatter<basic_string_view<T>, T>::format(buff.native(), ctx);
    }
};

static constexpr char _object_prefix(obj_type type)
{
    switch (type)
    {
    case obj_type::CLASS:
        return 'V';
    case obj_type::STRUCT:
        return 'U';
    default:
        assert(0 && "Wrong type");
        return 0;
    }
}

struct _type_info_pretty_name
{
    boost::typeindex::type_index info;
    mutable std::string          cached;

    template <typename C>
    chars_mixer<C, char> get() const
    {
        if (cached.empty())
            cached = info.pretty_name();
        return std::string_view(cached);
    }
};

template <typename T>
struct fmt::formatter<_type_info_pretty_name, T> : formatter<basic_string_view<T>, T>
{
    auto format(_type_info_pretty_name name, auto& ctx) const
    {
        return formatter<basic_string_view<T>, T>::format(name.get<T>().native(), ctx);
    }
};

struct _type_info_raw_name
{
    boost::typeindex::type_index info;

    template <typename C>
    chars_mixer<C, char> get() const
    {
        return info.raw_name();
    }
};

template <typename T>
struct fmt::formatter<_type_info_raw_name, T> : formatter<basic_string_view<T>, T>
{
    auto format(_type_info_raw_name name, auto& ctx) const
    {
        return formatter<basic_string_view<T>, T>::format(name.get<T>().native(), ctx);
    }
};

static void _log_found_vtable(library_info_name entry, std::type_info const& info, void const* vtablePtr)
{
    if (vtablePtr)
        spdlog::info(
            L"{} -> vtable for {} ({}) found! ({:p})",
            entry,
            _type_info_pretty_name(info),
            _type_info_raw_name(info),
            vtablePtr);
    else
        spdlog::warn(
            L"{} -> vtable for {} ({}) NOT found!",
            entry,
            entry,
            _type_info_pretty_name(info),
            _type_info_raw_name(info));
}

static void _log_found_vtable(
    library_info_name          entry,
    obj_type                   objType,
    chars_mixer<wchar_t, char> name,
    void const*                vtablePtr)
{
    if (vtablePtr)
        spdlog::info(L"{} -> vtable for {} '{}' found! ({:p})", entry, objType, name, vtablePtr);
    else
        spdlog::warn(L"{} -> vtable for {} '{}' NOT found!", entry, objType, name);
}

static void _log_found_vtable(library_info_name entry, chars_mixer<wchar_t, char> name, void const* vtablePtr)
{
    if (vtablePtr)
        spdlog::info(L"{} -> vtable for {} found! ({:p})", entry, name, vtablePtr);
    else
        spdlog::warn(L"{} -> vtable for {} NOT found!", entry, name);
}

namespace fd
{
static LDR_DATA_TABLE_ENTRY* _find_library(PVOID baseAddress)
{
    for (auto* e : ldr_tables_view_ptr())
    {
        if (baseAddress == e->DllBase)
            return e;
    }
    return nullptr;
}

static LDR_DATA_TABLE_ENTRY* _find_library(std::wstring_view name)
{
    for (auto* e : ldr_tables_view_ptr())
    {
        if (e->FullDllName.Buffer && name == library_info_name(e))
            return e;
    }
    return nullptr;
}

//------------

library_info::library_info(pointer entry)
    : entry_(entry)
{
}

bool library_info::is_root() const
{
    return entry_ == ldr_tables_view().begin();
}

bool library_info::unload() const
{
    return FreeLibrary(static_cast<HMODULE>(entry_->DllBase)) != 0;
}

auto library_info::get() const -> pointer
{
    return entry_;
}

auto library_info::operator->() const -> pointer
{
    return entry_;
}

auto library_info::operator*() const -> reference
{
    return *entry_;
}

std::wstring_view library_info::path() const
{
    return _library_info_path(entry_);
}

std::wstring_view library_info::name() const
{
    return library_info_name(entry_);
}

// void library_info::log_class_info(std::string_view rawName, const void* addr) const
//{
//     _log_address_found(entry_, rawName, addr);
// }

hidden_ptr library_info::find_export(std::string_view name) const
{
    auto exportPtr = _find_export(entry_, name);
    _log_found_object(entry_, L"export", name, exportPtr);
    return exportPtr;
}

IMAGE_SECTION_HEADER* library_info::find_section(std::string_view name) const
{
    auto header = _find_section(entry_, name);
    _log_found_object(entry_, L"section", name, header);
    return header;
}

hidden_ptr library_info::find_signature(std::string_view sig) const
{
    auto finder = pattern_scanner(_memory(entry_));
    auto result = finder(sig).front();

    _log_found_object(entry_, L"signature", sig, result);
    return result;
}

static bool _validate_rtti_name(char const* begin, std::string_view name)
{
    auto nameBegin = begin + 3 + 1;
    if (memcmp(nameBegin, name.data(), name.size()) != 0)
        return false;
    if (memcmp(nameBegin + name.size(), "@@", 2) != 0)
        return false;
    return true;
}

template <obj_type>
struct _find_type_descriptor;

template <>
struct _find_type_descriptor<obj_type::UNKNOWN>
{
    void* operator()(LDR_DATA_TABLE_ENTRY* entry, std::string_view name) const
    {
        auto scanner = pattern_scanner_raw(_memory(entry));
        for (auto begin : scanner(".?A", 3))
        {
            if (_validate_rtti_name(static_cast<char const*>(begin), name))
                return begin;
        }
        return nullptr;
    }
};

template <char Prefix>
struct _find_type_descriptor_known
{
    void* operator()(LDR_DATA_TABLE_ENTRY* entry, std::string_view name) const
    {
        auto scanner = pattern_scanner_raw(_memory(entry));
        for (auto begin : scanner(".?A", 3))
        {
            auto cBegin = static_cast<char const*>(begin);
            if (cBegin[3] == Prefix && _validate_rtti_name(cBegin, name))
                return begin;
        }
        return nullptr;
    }
};

template <>
struct _find_type_descriptor<obj_type::STRUCT> : _find_type_descriptor_known<_object_prefix(obj_type::STRUCT)>
{
};

template <>
struct _find_type_descriptor<obj_type::CLASS> : _find_type_descriptor_known<_object_prefix(obj_type::CLASS)>
{
};

static void* _find_vtable(
    IMAGE_SECTION_HEADER const* dotRdata,
    IMAGE_SECTION_HEADER const* dotText,
    dos_header                  dos,
    void const*                 rttiClassName)
{
    // get rtti type descriptor
    auto typeDescriptor = reinterpret_cast<uintptr_t>(rttiClassName);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    typeDescriptor -= sizeof(uintptr_t) * 2;

    auto dotRdataFinder = xrefs_scanner(dos + dotRdata->VirtualAddress, dotRdata->SizeOfRawData);
    auto dotTextFinder  = xrefs_scanner(dos + dotText->VirtualAddress, dotText->SizeOfRawData);

    for (auto xref : dotRdataFinder(typeDescriptor))
    {
        auto val          = reinterpret_cast<uint32_t>(xref);
        // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
        auto vtableOffset = *reinterpret_cast<uint32_t*>(val - 0x8);
        if (vtableOffset != 0)
            continue;

        auto objectLocator = val - 0xC;
        auto vtableAddress = reinterpret_cast<uintptr_t>(dotRdataFinder(objectLocator).front()) + 0x4;

        // check is valid offset
        assert(vtableAddress > sizeof(uintptr_t));
        return dotTextFinder(vtableAddress).front();
    }
    return nullptr;
}

static void* _find_vtable(LDR_DATA_TABLE_ENTRY* entry, void const* rttiClassName)
{
    auto dos      = _get_dos(entry);
    auto nt       = _get_nt(dos);
    auto sections = _sections(nt);

    return _find_vtable(_find_section(sections, ".rdata"), _find_section(sections, ".text"), dos, rttiClassName);
}

hidden_ptr library_info::find_vtable(std::string_view name) const
{
    union
    {
        void* typeDescriptorVoid;
        char* typeDescriptor;
    };

    static constexpr auto struct_ = std::string_view("struct ");
    static constexpr auto class_  = std::string_view("class ");

    if (name.starts_with(struct_))
    {
        constexpr _find_type_descriptor<obj_type::STRUCT> finder;
        typeDescriptorVoid = finder(entry_, name.substr(struct_.size()));
    }
    else if (name.starts_with(class_))
    {
        constexpr _find_type_descriptor<obj_type::CLASS> finder;
        typeDescriptorVoid = finder(entry_, name.substr(class_.size()));
    }
    else
    {
        assert(!name.contains(' '));
        constexpr _find_type_descriptor<obj_type::UNKNOWN> finder;
        typeDescriptorVoid = finder(entry_, name);
    }

    auto ptr = _find_vtable(entry_, typeDescriptorVoid);
    _log_found_vtable(entry_, typeDescriptor, ptr);
    return ptr;
}

hidden_ptr library_info::find_vtable(std::type_info const& info) const
{
    auto tindex = boost::typeindex::type_index(info);
    auto ptr    = _find_vtable(entry_, tindex.raw_name());
    _log_found_vtable(entry_, info, ptr);
    return ptr;
}

bool operator==(library_info info, std::nullptr_t)
{
    return info.get() == nullptr;
}

bool operator==(library_info info, library_info other)
{
    return info.get() == other.get();
}

bool operator==(library_info info, PVOID baseAddress)
{
    return info->DllBase == baseAddress;
}

bool operator==(library_info info, std::wstring_view name)
{
    return library_info_name(info.get()) == name;
}

bool operator!(library_info info)
{
    return !info.get();
}

library_info find_library(PVOID baseAddress)
{
    return _find_library(baseAddress);
}

library_info find_library(std::wstring_view name)
{
    return _find_library(name);
}

csgo_library_info::csgo_library_info(library_info info)
    : library_info(info)
{
}

hidden_ptr csgo_library_info::find_interface(std::string_view name) const
{
    return find_interface(_find_export(entry_, "CreateInterface"), name);
}

static bool _all_digits(char const* ptr)
{
    for (; *ptr != '\0'; ++ptr)
    {
        if (!std::isdigit(*ptr))
            return false;
    }
    return true;
}

hidden_ptr csgo_library_info::find_interface(void const* createInterfaceFn, std::string_view name) const
{
    assert(createInterfaceFn != nullptr);

    using iterator       = valve_ifc_reg_iterator<valve_ifc_reg_iterator_mode::compare>;
    using const_iterator = valve_ifc_reg_iterator<valve_ifc_reg_iterator_mode::const_compare>;

    auto target = std::find<iterator>(_root_interface(createInterfaceFn), nullptr, name);
    assert(target != nullptr);

    auto type = target.status();
    if (type == valve_ifc_reg_cmp_result::partial)
    {
        assert(_all_digits(target->name() + name.size()));
        assert(std::find<const_iterator>(target + 1, nullptr, name) == nullptr);
    }

    _log_found_csgo_interface(entry_, type, name, target.operator->());
    return (*std::as_const(target))();
}

static library_info _Current = nullptr;

static DECLSPEC_NOINLINE PVOID _self_module_handle()
{
    MEMORY_BASIC_INFORMATION info;
    constexpr SIZE_T         infoSize = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] auto    len      = VirtualQueryEx(GetCurrentProcess(), _self_module_handle, &info, infoSize);
    assert(len == infoSize);
    return static_cast<HINSTANCE>(info.AllocationBase);
}

static void _set_current_library()
{
    _Current = _find_library(_self_module_handle());
}

static void _set_current_library(HMODULE handle)
{
    assert(handle == _self_module_handle());
    _Current = _find_library(handle);
}

library_info current_library_info()
{
    if (!_Current)
        _set_current_library();
    else
        _log_found_entry(_Current.name(), _Current.get());

    return _Current;
}

void set_current_library(HMODULE const handle)
{
    _set_current_library(handle);
}

static void CALLBACK
_on_new_library(ULONG const notificationReason, PCLDR_DLL_NOTIFICATION_DATA const notificationData, PVOID const context)
{
    auto data = static_cast<library_info_cache*>(context);

    switch (notificationReason)
    {
    case LDR_DLL_NOTIFICATION_REASON_LOADED:
    {
        data->store(notificationData->Loaded.DllBase, _to_string_view(*notificationData->Loaded.BaseDllName));
        break;
    }
    case LDR_DLL_NOTIFICATION_REASON_UNLOADED:
    {
        data->remove(notificationData->Unloaded.DllBase, _to_string_view(*notificationData->Unloaded.BaseDllName));
        break;
    }
    default:
        assert(0 && "Unknown dll notification type");
        return;
    }

    // #if 0
    //         auto target_name = _To_string_view(*NotificationData->Loaded.FullDllName);
    //         if (!target_name.ends_with(data->name))
    //             return;
    // #else
    //         if (_to_string_view(*notificationData->Loaded.BaseDllName) != data->name)
    //             return;
    // #endif

    (void)0;
}

library_info_cache::cached_data::cached_data(std::wstring_view name)
    : name(name)
    , sem(0)
    , value(nullptr)
{
}

library_info_cache::cached_data::cached_data(LDR_DATA_TABLE_ENTRY* value, std::wstring_view name)
    : sem(1)
    , value(value)
{
    if (name.empty())
        std::construct_at(&this->name, library_info_name(value));
    else
    {
        auto path = _library_info_path(value);
        std::construct_at(&this->name, path.end() - name.size(), path.end());
    }
}

struct _dll_notification_funcs
{
    LdrRegisterDllNotification   reg;
    LdrUnregisterDllNotification unreg;
};

static struct : std::mutex, std::optional<_dll_notification_funcs>
{
} _LibraryInfoNotification;

library_info_cache::~library_info_cache()
{
    if (cookie_ && !NT_SUCCESS(_LibraryInfoNotification->unreg(cookie_)))
        assert(0 && "Unable to remove dll notification");

    std::for_each(cache_.rbegin(), cache_.rend(), [](auto& data) { data->sem.release(); });
}

library_info_cache::library_info_cache()
{
    if (!_LibraryInfoNotification)
    {
        auto g = std::lock_guard(_LibraryInfoNotification);
        if (!_LibraryInfoNotification)
        {
            auto ntdll = _find_library(L"ntdll.dll");
            assert(ntdll != nullptr);
            auto exports = dll_exports(ntdll);

            auto& n = _LibraryInfoNotification.emplace();
            n.reg   = reinterpret_cast<LdrRegisterDllNotification>(_find_export(exports, "LdrRegisterDllNotification"));
            n.unreg = reinterpret_cast<LdrUnregisterDllNotification>(
                _find_export(exports, "LdrUnregisterDllNotification"));
        }
    }

    auto g = std::lock_guard(mtx_);

    if (!NT_SUCCESS(_LibraryInfoNotification->reg(0, _on_new_library, this, &cookie_)))
    {
        assert(0 && "Unable to create dll notification");
        terminate();
    }

    for (auto entry : ldr_tables_view_ptr())
        cache_.emplace_back(std::make_unique<cached_data>(entry));
}

void library_info_cache::store(PVOID baseAddress, std::wstring_view name)
{
    auto g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    auto entry = _find_library(baseAddress);
    for (auto& data : cache_)
    {
        if (data->name == name)
        {
            assert(data->value == nullptr);
            data->value = entry;
            data->sem.release();
            return;
        }
    }

    // if (find(cache_, baseAddress) == _end(cache_))
    cache_.emplace_back(std::make_unique<cached_data>(entry));
}

void library_info_cache::remove(PVOID baseAddress, std::wstring_view name)
{
    auto g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    auto& val = *std::find_if(
        cache_.rbegin(),
        cache_.rend(),
        [&](auto& data)
        {
            //
            return data->value && data->value->DllBase == baseAddress;
        });

    val->sem.release();
    std::swap(cache_.back(), val);
    cache_.pop_back();
}

library_info library_info_cache::get(PVOID baseAddress) const
{
    auto g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    for (auto& data : cache_)
    {
        if (data->value && data->value->DllBase == baseAddress)
        {
            _log_found_entry(baseAddress, data->value);
            return data->value;
        }
    }

    _log_found_entry(baseAddress, nullptr);
    return nullptr;
}

library_info library_info_cache::get(std::wstring_view name) const
{
    auto g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    for (auto& data : cache_)
    {
        if (data->name == name)
        {
            assert(data->value != nullptr);
            _log_found_entry(name, data->value);
            return data->value;
        }
    }

    _log_found_entry(name, nullptr);
    return nullptr;
}

library_info library_info_cache::get(std::wstring_view name)
{
    cached_data* newData;
    {
        auto g = std::lock_guard(mtx_);
        assert(cookie_ != nullptr);

        for (auto& data : cache_)
        {
            if (data->name == name)
            {
                _log_found_entry_or_wait(name, data->value);
                if (!data->value)
                    break;
                return data->value;
            }
        }
        newData = cache_.emplace_back(std::make_unique<cached_data>(name)).get();
    }
    newData->sem.acquire();
    auto g = std::lock_guard(mtx_);
    _log_found_entry(name, newData->value);
    return newData->value;
}
} // namespace fd