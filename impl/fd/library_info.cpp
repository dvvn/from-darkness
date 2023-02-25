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
    auto const size = left.size();
    return size == right.size() && std::char_traits<T>::compare(left.data(), right.data(), size) == 0;
}

template <typename T>
static bool operator==(fmt::basic_string_view<T> left, std::basic_string_view<T> right)
{
    auto const size = left.size();
    return size == right.size() && std::char_traits<T>::compare(left.data(), right.data(), size) == 0;
}

template <typename ContextC, typename StrC, bool Same = std::same_as<ContextC, StrC>>
class _chars_mixer;

template <typename ContextC, typename StrC>
class _chars_mixer<ContextC, StrC, true>
{
    fmt::basic_string_view<StrC> cached_;

  public:
    _chars_mixer(std::basic_string_view<StrC> str = {})
        : cached_(str)
    {
    }

    auto native() const
    {
        return cached_;
    }
};

template <typename ContextC, typename StrC>
class _chars_mixer<ContextC, StrC, false>
{
    using context_str = fmt::basic_string_view<ContextC>;
    using current_str = fmt::basic_string_view<StrC>;

    using current_str_in = std::basic_string_view<StrC>;

    current_str cached_;

  public:
    static_assert(sizeof(ContextC) >= sizeof(StrC));

    _chars_mixer() = default;

    _chars_mixer(StrC const* ptr)
        : cached_(ptr)
    {
    }

    _chars_mixer(current_str_in str)
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
        buffer(const current_str str)
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
struct fmt::formatter<_chars_mixer<ContextC, StrC>, ContextC> : formatter<basic_string_view<ContextC>, ContextC>
{
    auto format(_chars_mixer<ContextC, StrC> const& mixer, auto& ctx) const
    {
        return formatter<basic_string_view<ContextC>, ContextC>::format(mixer.native(), ctx);
    }
};

using _lazy_wstring = _chars_mixer<wchar_t, char>;

enum interface_reg_iterator_mode : uint8_t
{
    normal,
    compare,
    const_compare
};

class interface_reg
{
    template <interface_reg_iterator_mode>
    friend class interface_reg_iterator;

    void* (*createFn_)();
    char const*    name_;
    interface_reg* next_;

  public:
#if 0
    class range
    {
        interface_reg* root_;

      public:
        range(void const* createInterfaceFn)
        {
            auto const relativeFn   = reinterpret_cast<uintptr_t>(createInterfaceFn) + 0x5;
            auto const displacement = *reinterpret_cast<int32_t*>(relativeFn);
            auto const jmp          = relativeFn + sizeof(int32_t) + displacement;

            root_ = **reinterpret_cast<interface_reg***>(jmp + 0x6);
        }

        range(interface_reg* current)
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

    interface_reg()                     = delete;
    interface_reg(interface_reg const&) = delete;

    void* operator()() const
    {
        return createFn_();
    }

    char const* name() const
    {
        return name_;
    }

    interface_reg const* operator+(size_t offset) const
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

enum interface_reg_cmp_result : uint8_t
{
    error,
    full,
    partial,
    unset
};

template <interface_reg_iterator_mode Mode>
class interface_reg_iterator
{
    using cmp_type =
        std::conditional_t<Mode == interface_reg_iterator_mode::normal, std::false_type, interface_reg_cmp_result>;

    interface_reg*                 current_;
    [[no_unique_address]] cmp_type compared_;

    template <interface_reg_iterator_mode>
    friend class interface_reg_iterator;

  public:
    interface_reg_iterator(interface_reg_iterator const&) = default;

    template <interface_reg_iterator_mode M>
    interface_reg_iterator(interface_reg_iterator<M> other)
        : current_(other.current_)
        , compared_(other.compared_)
    {
        if constexpr (Mode == interface_reg_iterator_mode::const_compare)
            assert(compared_ != error);
    }

    interface_reg_iterator(interface_reg* reg)
        : current_(reg)
        , compared_(unset)
    {
    }

    interface_reg_iterator& operator++()
    {
        current_ = current_->next_;
        return *this;
    }

    interface_reg_iterator operator++(int)
    {
        auto const tmp = current_;
        current_       = current_->next_;
        return tmp;
    }

    interface_reg_iterator& operator*() requires(Mode != normal)
    {
        return *this;
    }

    void set(interface_reg_cmp_result status) requires(Mode != normal)
    {
        compared_ = status;
    }

    interface_reg const& operator*() const
    {
        return *this->current_;
    }

    interface_reg* operator->() const
    {
        return current_;
    }

    template <interface_reg_iterator_mode M>
    bool operator==(interface_reg_iterator<M> const other) const
    {
        return current_ == other.current_;
    }

    bool operator==(std::nullptr_t) const
    {
        return !current_;
    }

    interface_reg_iterator operator+(size_t i) const
    {
        assert(i == 1);
        auto ret = interface_reg_iterator(current_->next_);
        if constexpr (Mode == interface_reg_iterator_mode::const_compare)
            ret.compared_ = compared_;
        return ret;
    }

    interface_reg_cmp_result status() const
    {
        return compared_;
    }
};

template <interface_reg_iterator_mode Mode>
struct std::iterator_traits<interface_reg_iterator<Mode>>
{
    using type       = std::forward_iterator_tag;
    using value_type = interface_reg;
};

bool operator==(interface_reg_iterator<compare>& it, const std::string_view ifcName)
{
    auto&      curr     = *std::as_const(it);
    auto const currName = std::string_view(curr.name());

    auto cmp = interface_reg_cmp_result::error;
    if (currName.starts_with(ifcName))
    {
        if (currName.size() == ifcName.size())
            cmp = full;
        else if (std::isdigit(currName[ifcName.size()]))
            cmp = partial;
    }
    it.set(cmp);
    return cmp != error;
}

bool operator==(interface_reg_iterator<const_compare> const& it, const std::string_view ifcName)
{
    switch (it.status())
    {
    case full:
    {
        return it->name() == ifcName;
    }
    case partial:
    {
        auto const currName = std::string_view(it->name());
        if (currName.size() <= ifcName.size())
            return false;
        if (!std::isdigit(currName[ifcName.size()]))
            return false;
        return currName.starts_with(ifcName);
    }
    default:
    {
        assert("Wrong state");
        return false;
    }
    }
}

static interface_reg* _root_interface(void const* createInterfaceFn)
{
    auto const relativeFn   = reinterpret_cast<uintptr_t>(createInterfaceFn) + 0x5;
    auto const displacement = *reinterpret_cast<int32_t*>(relativeFn);
    auto const jmp          = relativeFn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<interface_reg***>(jmp + 0x6);
}

template <typename T>
struct fmt::formatter<interface_reg, T> : formatter<basic_string_view<T>, T>
{
    auto format(interface_reg const& reg, auto& ctx) const
    {
        auto const tmp = _chars_mixer<T, char>(reg.name());
        return formatter<basic_string_view<T>, T>::format(tmp.native(), ctx);
    }
};

//---------

struct _dos_entry
{
    uintptr_t addr;

    template <typename T>
    operator T*() const
    {
        return reinterpret_cast<T*>(addr);
    }
};

class _dos_header
{
    const IMAGE_DOS_HEADER* dos_;

  public:
    _dos_header(const IMAGE_DOS_HEADER* dos)
        : dos_(dos)
    {
    }

    const IMAGE_DOS_HEADER* operator->() const
    {
        return dos_;
    }

    const IMAGE_DOS_HEADER& operator*() const
    {
        return *dos_;
    }

    _dos_entry operator+(ptrdiff_t offset) const
    {
        return { reinterpret_cast<uintptr_t>(dos_) + offset };
    }
};

static IMAGE_DOS_HEADER* _get_dos(const LDR_DATA_TABLE_ENTRY* entry)
{
    auto const dos = static_cast<IMAGE_DOS_HEADER*>(entry->DllBase);
    // check for invalid DOS / DOS signature.
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    return dos;
}

static IMAGE_NT_HEADERS* _get_nt(_dos_header dos)
{
    IMAGE_NT_HEADERS* nt = dos + dos->e_lfanew;
    // check for invalid NT / NT signature.
    assert(nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
    return nt;
}

static IMAGE_NT_HEADERS* _get_nt(const LDR_DATA_TABLE_ENTRY* entry)
{
    return _get_nt(_get_dos(entry));
}

static std::span<IMAGE_SECTION_HEADER> _sections(const IMAGE_NT_HEADERS* nt)
{
    return { IMAGE_FIRST_SECTION(nt), static_cast<size_t>(nt->FileHeader.NumberOfSections) };
}

static std::span<IMAGE_SECTION_HEADER> _sections(const LDR_DATA_TABLE_ENTRY* entry)
{
    return _sections(_get_nt(entry));
}

static std::span<uint8_t> _memory(const LDR_DATA_TABLE_ENTRY* entry, const IMAGE_NT_HEADERS* nt = nullptr)
{
    if (!nt)
        nt = _get_nt(entry);
    //(void)nt->OptionalHeader.BaseOfCode;
    //(void)nt->OptionalHeader.BaseOfData;
    //(void)nt->OptionalHeader.ImageBase; //same as entry->DllBase
    return { static_cast<uint8_t*>(entry->DllBase), nt->OptionalHeader.SizeOfImage };
}

static IMAGE_SECTION_HEADER* _find_section(const std::span<IMAGE_SECTION_HEADER> rng, const std::string_view name)
{
    for (auto& h : rng)
    {
        if (reinterpret_cast<char const*>(h.Name) == name)
            return &h;
    }
    return nullptr;
}

static IMAGE_SECTION_HEADER* _find_section(const LDR_DATA_TABLE_ENTRY* entry, const std::string_view name)
{
    return _find_section(_sections(entry), name);
}

class _exports
{
    _dos_header dos_;

    uint32_t* names_;
    uint32_t* funcs_;
    uint16_t* ords_;

    union
    {
        IMAGE_EXPORT_DIRECTORY* exportDirDir_;
        uint8_t*                virtualAddrStart_;
    };

    uint8_t* virtualAddrEnd_;

    using src_ptr = _exports const*;

  public:
    _exports(const IMAGE_DOS_HEADER* dos, const IMAGE_NT_HEADERS* nt = 0)
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

    _exports(const _LDR_DATA_TABLE_ENTRY* entry)
        : _exports(_get_dos(entry))
    {
    }

    char const* name(DWORD offset) const
    {
        return dos_ + names_[offset];
    }

    void* function(DWORD offset) const
    {
        auto const tmp = dos_ + funcs_[ords_[offset]];
        if (tmp < virtualAddrStart_ || tmp >= virtualAddrEnd_)
            return tmp;

        // todo:resolve fwd export
        assert("Forwarded export detected");
        return nullptr;

#if 0
		// get forwarder std::string.
		const std::string_view fwd_str = export_ptr.get<const char*>( );

		// forwarders have a period as the delimiter.
		const auto delim = fwd_str.find_last_of('.');
		if(delim == fwd_str.npos)
			continue;

		using namespace string_view_literals;
		// get forwarder mod name.
		const info_string::fixed_type fwd_module_str = nstd::append<std::wstring>(fwd_str.substr(0, delim), L".dll"sv);

		// get real export ptr ( recursively ).
		const auto target_module = std::ranges::find_if(*all_modules, [&](const info& i)
		{
			return i.name == fwd_module_str;
		});
		if(target_module == all_modules->end( ))
			continue;

		// get forwarder export name.
		const auto fwd_export_str = fwd_str.substr(delim + 1);

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

        return nullptr;
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

static void* _find_export(_exports const& exports, const std::string_view name)
{
    for (auto const val : exports)
    {
        if (val.name() == name)
            return val.function();
    }
    return nullptr;
}

template <typename T, bool Deref = true>
class win_list_view
{
    using pointer   = const LIST_ENTRY*;
    using reference = const LIST_ENTRY&;

    pointer root_;

  public:
    class iterator
    {
        pointer current_;

      public:
        iterator(const pointer ptr = nullptr)
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
            auto const c = current_;
            current_     = current_->Flink;
            return c;
        }

        iterator& operator--()
        {
            current_ = current_->Blink;
            return *this;
        }

        iterator operator--(int)
        {
            auto const c = current_;
            current_     = current_->Blink;
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

    win_list_view(const LIST_ENTRY* root = nullptr)
    {
        if (root)
        {
            root_ = root;
            return;
        }

#if defined(_M_X64) || defined(__x86_64__)
        auto const mem = NtCurrentTeb();
        auto const ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
        auto const mem = reinterpret_cast<PEB*>(__readfsdword(0x30));
        auto const ldr = mem->Ldr;
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

static void _to_string_view(const LDR_DATA_TABLE_ENTRY* entry) = delete;

static auto _to_string_view(const UNICODE_STRING& ustr)
{
    std::wstring_view ret;
    if (ustr.Buffer)
        std::construct_at(&ret, ustr.Buffer, ustr.Length / sizeof(WCHAR));
    return ret;
}

static auto _to_string(const UNICODE_STRING& ustr)
{
    return std::wstring(_to_string_view(ustr));
}

static auto _library_info_path(const LDR_DATA_TABLE_ENTRY* entry)
{
    return _to_string_view(entry->FullDllName);
}

class _library_info_name
{
    const LDR_DATA_TABLE_ENTRY* entry;

  public:
    _library_info_name(const LDR_DATA_TABLE_ENTRY* entry)
        : entry(entry)
    {
    }

    operator std::wstring_view() const
    {
        auto const fullPath = _library_info_path(entry);
#if 0
    return std::wstring_view(file_view(fullPath).name());
#else
        auto const nameStart = fullPath.rfind('\\');
        assert(nameStart != fullPath.npos);
        return fullPath.substr(nameStart + 1);
#endif
    }
};

template <typename T>
struct fmt::formatter<_library_info_name, T> : formatter<basic_string_view<T>, T>
{
    auto format(_library_info_name name, auto& ctx) const
    {
        auto const tmp = _chars_mixer<T, wchar_t>(name);
        return formatter<basic_string_view<T>, T>::format(tmp.native(), ctx);
    }
};

static void _log_found_entry(const std::wstring_view name, const /*LDR_DATA_TABLE_ENTRY*/ void* entry)
{
    if (entry)
        spdlog::info(L"{} -> found! ({:p})", name, entry);
    else
        spdlog::warn(L"{} -> NOT found!", name);
}

static void _log_found_entry(const /*IMAGE_DOS_HEADER*/ void* baseAddress, const LDR_DATA_TABLE_ENTRY* entry)
{
    if (entry)
        spdlog::info(L"{:p} ({}) -> found! ({:p})", baseAddress, _library_info_name(entry), fmt::ptr(entry));
    else
        spdlog::warn(L"{:p} -> NOT found!", baseAddress);
}

// template <>
// struct fmt::formatter<_lazy_wstring, wchar_t> : fmt::formatter<std::wstring, wchar_t>
//{
//     template <typename FormatCtx>
//     auto format(_lazy_wstring str, FormatCtx& ctx) const
//     {
//         return formatter<std::wstring, wchar_t>::format(std::wstring(str), ctx);
//     }
// };

static auto _log_found_csgo_interface(
    _library_info_name       entry,
    interface_reg_cmp_result type,
    _lazy_wstring            name,
    interface_reg const*     reg)
{
    switch (type)
    {
    case error:
        spdlog::warn(L"{} -> csgo interface '{}' NOT found!", entry, name);
        break;
    case full:
        spdlog::info(L"{} -> csgo interface '{}' found! ({:p})", entry, name, fmt::ptr(reg));
        break;
    case partial:
        spdlog::info(L"{} -> csgo interface '{}' ({}) found! ({:p})", entry, name, *reg, fmt::ptr(reg));
        break;
    }
}

static auto _log_found_object(
    _library_info_name      entry,
    const std::wstring_view objectType,
    _lazy_wstring           object,
    void const*             addr)
{
    if (addr)
        spdlog::info(L"{} -> {} '{}' found! ({:p})", entry, objectType, object, addr);
    else
        spdlog::warn(L"{} -> {} '{}' NOT found!", entry, objectType, object);
}

// static void _log_address_found(_library_info_name entry, _object_type_fancy rawName, void const* addr)
//{
//     if (addr)
//         spdlog::info(L"{} -> {} found! ({:p})", entry, rawName, addr);
//     else
//         spdlog::warn(L"{} -> {} NOT found!", entry, rawName);
// }

#if 0

static constexpr auto _HexDigits = "0123456789ABCDEF";

template <typename T>
[[nodiscard]]
static constexpr auto _bytes_to_sig(T const* bytes, const size_t size)
{
    auto const hexLength = /*(size << 1) + size*/ size * 3;

    // construct pre-reserved std::string filled with spaces
    std::string pattern(hexLength - 1, ' ');

    for (size_t i = 0, n = 0; i < hexLength; ++n, i += 3)
    {
        const uint8_t currByte = bytes[n];

        // manually convert byte to chars
        pattern[i]     = _HexDigits[((currByte & 0xF0) >> 4)];
        pattern[i + 1] = _HexDigits[(currByte & 0x0F)];
    }

    return pattern;
}

template <typename T, typename T1>
static constexpr auto _bytes_to_sig(T* bytesOut, T1* bytes, const size_t size)
{
    // construct pre-reserved std::string filled with spaces
    // std::fill_n(bytesOut, size - 1, ' ');

    for (size_t i = 0, n = 0; i < size; ++n, i += 3)
    {
        const uint8_t currByte = bytes[n];

        // manually convert byte to chars
        bytesOut[i]     = _HexDigits[((currByte & 0xF0) >> 4)];
        bytesOut[i + 1] = _HexDigits[(currByte & 0x0F)];
    }

    return std::false_type();
}

static constexpr struct
{
    std::array<char, 3> rawPrefix  = { '.', '?', 'A' };
    std::array<char, 2> rawPostfix = { '@', '@' };

#define SPACES1 ' '
#define SPACES2 ' ', ' '
#define SPACES3 SPACES2, ' '

    std::array<uint8_t, 3 + 3 + 2> rawPrefixBytes{ SPACES3, SPACES3, SPACES2 };
    std::array<uint8_t, 2 + 2 + 1> rawPostfixBytes{ SPACES2, SPACES2, SPACES1 };

#undef SPACES1
#undef SPACES2
#undef SPACES3

    char classPrefix  = 'V';
    char structPrefix = 'U';

  private:
    [[no_unique_address]] std::false_type dummy1_ = _bytes_to_sig(rawPrefixBytes.data(), rawPrefix.data(), 3 * 3);
    [[no_unique_address]] std::false_type dummy2_ = _bytes_to_sig(rawPostfixBytes.data(), rawPostfix.data(), 2 * 3);
} _RttiInfo;

#endif

static std::string_view _object_type(char const* rawName)
{
    assert(memcmp(rawName, ".?A", 3) == 0);
    auto const prefix = *(rawName + 3);
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
        _chars_mixer<T, char> buff;

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
    default:;
        return *static_cast<char*>(nullptr);
    }
}

struct _type_info_pretty_name
{
    boost::typeindex::type_index info;
    mutable std::string          cached;

    template <typename C>
    _chars_mixer<C, char> get() const
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
    _chars_mixer<C, char> get() const
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

static void _log_found_vtable(_library_info_name entry, std::type_info const& info, void const* vtablePtr)
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
    _library_info_name          entry,
    obj_type                    objType,
    _chars_mixer<wchar_t, char> name,
    void const*                 vtablePtr)
{
    if (vtablePtr)
        spdlog::info(L"{} -> vtable for {} '{}' found! ({:p})", entry, objType, name, vtablePtr);
    else
        spdlog::warn(L"{} -> vtable for {} '{}' NOT found!", entry, objType, name);
}

static void _log_found_vtable(_library_info_name entry, _chars_mixer<wchar_t, char> name, void const* vtablePtr)
{
    if (vtablePtr)
        spdlog::info(L"{} -> vtable for {} found! ({:p})", entry, name, vtablePtr);
    else
        spdlog::warn(L"{} -> vtable for {} NOT found!", entry, name);
}

namespace fd
{
static const LDR_DATA_TABLE_ENTRY* _find_library(PVOID baseAddress)
{
    for (auto const* e : ldr_tables_view_ptr())
    {
        if (baseAddress == e->DllBase)
            return e;
    }
    return nullptr;
}

static const LDR_DATA_TABLE_ENTRY* _find_library(std::wstring_view name)
{
    for (auto const* e : ldr_tables_view_ptr())
    {
        if (e->FullDllName.Buffer && name == _library_info_name(e))
            return e;
    }
    return nullptr;
}

//------------

library_info::library_info(const pointer entry)
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
    return _library_info_name(entry_);
}

// void library_info::log_class_info(const std::string_view rawName, const void* addr) const
//{
//     _log_address_found(entry_, rawName, addr);
// }

void* library_info::find_export(const std::string_view name) const
{
    auto exportPtr = _find_export(entry_, name);
    _log_found_object(entry_, L"export", name, exportPtr);
    return exportPtr;
}

IMAGE_SECTION_HEADER* library_info::find_section(const std::string_view name) const
{
    auto header = _find_section(entry_, name);
    _log_found_object(entry_, L"section", name, header);
    return header;
}

void* library_info::find_signature(const std::string_view sig) const
{
    auto const finder = pattern_scanner(_memory(entry_));
    auto const result = finder(sig).front();

    _log_found_object(entry_, L"signature", sig, result);
    return result;
}

static bool _validate_rtti_name(char const* begin, const std::string_view name)
{
    auto const nameBegin = begin + 3 + 1;
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
    void* operator()(const LDR_DATA_TABLE_ENTRY* entry, const std::string_view name) const
    {
        auto const scanner = pattern_scanner_raw(_memory(entry));
        for (auto const begin : scanner(".?A", 3))
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
    void* operator()(const LDR_DATA_TABLE_ENTRY* entry, const std::string_view name) const
    {
        auto const scanner = pattern_scanner_raw(_memory(entry));
        for (auto const begin : scanner(".?A", 3))
        {
            auto const cBegin = static_cast<char const*>(begin);
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
    IMAGE_SECTION_HEADER* dotRdata,
    IMAGE_SECTION_HEADER* dotText,
    _dos_header           dos,
    void const*           rttiClassName)
{
    // get rtti type descriptor
    auto typeDescriptor = reinterpret_cast<uintptr_t>(rttiClassName);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    typeDescriptor -= sizeof(uintptr_t) * 2;

    auto const dotRdataFinder = xrefs_scanner(dos + dotRdata->VirtualAddress, dotRdata->SizeOfRawData);
    auto const dotTextFinder  = xrefs_scanner(dos + dotText->VirtualAddress, dotText->SizeOfRawData);

    for (auto const xref : dotRdataFinder(typeDescriptor))
    {
        auto const val          = reinterpret_cast<uint32_t>(xref);
        // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
        auto const vtableOffset = *reinterpret_cast<uint32_t*>(val - 0x8);
        if (vtableOffset != 0)
            continue;

        auto const objectLocator = val - 0xC;
        auto const vtableAddress = reinterpret_cast<uintptr_t>(dotRdataFinder(objectLocator).front()) + 0x4;

        // check is valid offset
        assert(vtableAddress > sizeof(uintptr_t));
        return dotTextFinder(vtableAddress).front();
    }
    return nullptr;
}

static void* _find_vtable(const LDR_DATA_TABLE_ENTRY* entry, void const* rttiClassName)
{
    auto dos      = _get_dos(entry);
    auto nt       = _get_nt(dos);
    auto sections = _sections(nt);

    return _find_vtable(_find_section(sections, ".rdata"), _find_section(sections, ".text"), dos, rttiClassName);
}

void* library_info::find_vtable(const std::string_view name) const
{
    union
    {
        void const* typeDescriptorVoid;
        char const* typeDescriptor;
    };

    if (name.starts_with("struct "))
    {
        constexpr _find_type_descriptor<obj_type::STRUCT> finder;
        typeDescriptorVoid = finder(entry_, name.substr(7));
    }
    else if (name.starts_with("class "))
    {
        constexpr _find_type_descriptor<obj_type::CLASS> finder;
        typeDescriptorVoid = finder(entry_, name.substr(6));
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

void* library_info::find_vtable(std::type_info const& info) const
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
    return _library_info_name(info.get()) == name;
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

void* csgo_library_info::find_interface(const std::string_view name) const
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

void* csgo_library_info::find_interface(void const* createInterfaceFn, const std::string_view name) const
{
    assert(createInterfaceFn != nullptr);

    using iterator       = interface_reg_iterator<compare>;
    using const_iterator = interface_reg_iterator<const_compare>;

    const_iterator const target = std::find<iterator>(_root_interface(createInterfaceFn), nullptr, name);
    assert(target != nullptr);

    auto const type = target.status();
    if (type == partial)
    {
        assert(_all_digits(target->name() + name.size()));
        assert(std::find<const_iterator>(target + 1, nullptr, name) == nullptr);
    }

    _log_found_csgo_interface(entry_, type, name, target.operator->());
    return (*target)();
}

static library_info _Current = nullptr;

static DECLSPEC_NOINLINE PVOID _self_module_handle()
{
    MEMORY_BASIC_INFORMATION    info;
    constexpr SIZE_T            infoSize = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] auto const len      = VirtualQueryEx(GetCurrentProcess(), _self_module_handle, &info, infoSize);
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

void set_current_library(const HMODULE handle)
{
    _set_current_library(handle);
}

void library_info_cache::release_delayed()
{
    for (auto& info : boost::adaptors::reverse(delayed_))
        info.sem.release();
}

static void CALLBACK
_on_new_library(const ULONG notificationReason, const PCLDR_DLL_NOTIFICATION_DATA notificationData, const PVOID context)
{
    auto const data = static_cast<library_info_cache*>(context);

    switch (notificationReason)
    {
    case LDR_DLL_NOTIFICATION_REASON_LOADED:
    {
        data->store(notificationData->Loaded.DllBase, _to_string_view(*notificationData->Loaded.BaseDllName));
        break;
    }
    case LDR_DLL_NOTIFICATION_REASON_UNLOADED:
    {
        data->remove(notificationData->Unloaded.DllBase, _to_string(*notificationData->Unloaded.BaseDllName));
        break;
    }
    default:
        assert("Unknown dll notification type");
        return;
    }

    // #if 0
    //         const auto target_name = _To_string_view(*NotificationData->Loaded.FullDllName);
    //         if (!target_name.ends_with(data->name))
    //             return;
    // #else
    //         if (_to_string_view(*notificationData->Loaded.BaseDllName) != data->name)
    //             return;
    // #endif

    (void)0;
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
    if (!cookie_)
        return;

    if (!NT_SUCCESS(_LibraryInfoNotification->unreg(cookie_)))
    {
        assert("Unable to remove dll notification");
        return;
    }

    if (!delayed_.empty())
    {
        cache_.clear();
        release_delayed();
    }
}

library_info_cache::library_info_cache()
{
    if (!_LibraryInfoNotification)
    {
        auto const g = std::lock_guard(_LibraryInfoNotification);
        if (!_LibraryInfoNotification)
        {
            auto const ntdll = _find_library(L"ntdll.dll");
            assert(ntdll != nullptr);
            auto const exports = _exports((ntdll));

            auto& n = _LibraryInfoNotification.emplace();
            n.reg   = reinterpret_cast<LdrRegisterDllNotification>(_find_export(exports, "LdrRegisterDllNotification"));
            n.unreg = reinterpret_cast<LdrUnregisterDllNotification>(
                _find_export(exports, "LdrUnregisterDllNotification"));
        }
    }

    auto const g = std::lock_guard(mtx_);

    if (!NT_SUCCESS(_LibraryInfoNotification->reg(0, _on_new_library, this, &cookie_)))
    {
        assert("Unable to create dll notification");
        terminate();
    }

    for (auto entry : ldr_tables_view_ptr())
        cache_.emplace_back(entry);
    cache_.shrink_to_fit();
}

void library_info_cache::store(PVOID baseAddress, std::wstring_view name)
{
    auto const g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    // if (find(cache_, baseAddress) == _end(cache_))
    cache_.emplace_back(_find_library(baseAddress));

    for (auto it = delayed_.begin(); it != delayed_.end(); ++it)
    {
        if (it->name == name)
        {
            it->sem.release();
            delayed_.erase(it);
            break;
        }
    }
}

void library_info_cache::remove(PVOID baseAddress, std::wstring name)
{
    auto const g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    auto& last = cache_.back();
    auto  info = std::find(cache_.begin(), cache_.end() - 1, baseAddress);
    using std::swap;
    swap(*info, last);
    assert(last == baseAddress);
    cache_.pop_back();
}

static auto _try_get_from_cache(auto& cache, auto val)
{
    library_info result;
    for (auto& info : cache)
    {
        if (info == val)
        {
            result = info;
            break;
        }
    }
    _log_found_entry(val, result.get());
    return result;
}

library_info library_info_cache::get(PVOID baseAddress) const
{
    auto const g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);
    return _try_get_from_cache(cache_, baseAddress);
}

library_info library_info_cache::get(std::wstring_view name) const
{
    auto const g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);
    return _try_get_from_cache(cache_, name);
}

static bool operator==(_delayed_library_info const& info, const std::wstring_view name)
{
    return info.name == name;
}

library_info library_info_cache::get(std::wstring_view name)
{
    std::binary_semaphore* sem;
    {
        auto const g = std::lock_guard(mtx_);
        assert(cookie_ != nullptr);

        auto cached = _try_get_from_cache(cache_, name);
        if (cached != nullptr)
            return cached;

        auto info = std::find(delayed_.begin(), delayed_.end(), name);
        if (info == delayed_.end())
            sem = &delayed_.emplace_back(name).sem;
        else
            sem = &info->sem;
    }

    sem->acquire();
    return _try_get_from_cache(cache_, name);
}

void library_info_cache::destroy()
{
    auto const g = std::lock_guard(mtx_);
    assert(cookie_ != nullptr);

    if (!NT_SUCCESS(_LibraryInfoNotification->unreg(cookie_)))
    {
        assert("Unable to remove dll notification");
        terminate();
    }

    cache_.clear();
    release_delayed();
    delayed_.clear();
    cookie_ = nullptr;
}
} // namespace fd