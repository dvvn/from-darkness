// ReSharper disable CppClangTidyClangDiagnosticMicrosoftCast
#include <fd/library_info.h>
#include <fd/mem_scanner.h>

#include <spdlog/spdlog.h>

#include <boost/core/demangle.hpp>

#include <array>
#include <semaphore>
#include <span>

#define FD_ASSERT(...)

template <typename T>
static bool operator==(std::basic_string_view<T> left, fmt::basic_string_view<T> right)
{
    const auto size = left.size();
    return size == right.size() && std::char_traits<T>::compare(left.data(), right.data(), size);
}

template <typename T>
static bool operator==(fmt::basic_string_view<T> left, std::basic_string_view<T> right)
{
    const auto size = left.size();
    return size == right.size() && std::char_traits<T>::compare(left.data(), right.data(), size);
}

template <typename T>
struct any_string_view
{
    T obj;

    operator fmt::basic_string_view<std::iter_value_t<decltype(obj.data())>>() const
    {
        return { obj.data(), obj.size() };
    }
};

template <typename T>
any_string_view(T&&) -> any_string_view<std::remove_cv_t<T>>;

template <typename ContextC, typename StrC, bool Same = std::same_as<ContextC, StrC>>
class _chars_mixer;

template <typename ContextC, typename StrC>
class _chars_mixer<ContextC, StrC, true>
{
    fmt::basic_string_view<StrC> cached_;

  public:
    _chars_mixer(std::basic_string_view<StrC> str)
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
    _chars_mixer(const StrC* ptr)
        : cached_(ptr)
    {
        static_assert(sizeof(ContextC) >= sizeof(StrC));
    }

    _chars_mixer(current_str_in str)
        : cached_(str)
    {
        static_assert(sizeof(ContextC) >= sizeof(StrC));
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
    template <class FormatContext>
    auto format(const _chars_mixer<ContextC, StrC>& mixer, FormatContext& ctx) const
    {
        return formatter<basic_string_view<ContextC>, ContextC>::format(mixer.native(), ctx);
    }
};

class _object_type_fancy
{
    std::string_view str;

  public:
    _object_type_fancy(std::string_view str)
        : str(str)
    {
    }

    template <typename T>
    class converter
    {
        fmt::basic_memory_buffer<T> buff_;

      public:
        operator fmt::basic_string_view<T>() const
        {
            return { buff_.data(), buff_.size() };
        }

        converter(std::string_view str)
        {
            const auto nameBegin = str.find(' '); // class or struct
            if (nameBegin == std::string_view::npos)
            {
                constexpr std::string_view obj = "object ";
                buff_.append(obj);
            }
            else
            {
                auto obj = str.substr(nameBegin);
                buff_.append(obj);
                str.remove_prefix(nameBegin + 1);
            }
            buff_.push_back('\'');
            buff_.append(str);
            buff_.push_back('\'');
        }
    };

    template <typename T>
    converter<T> fancy() const
    {
        return str;
    }
};

template <typename ContextC>
struct fmt::formatter<_object_type_fancy, ContextC> : formatter<basic_string_view<ContextC>, ContextC>
{
    template <class FormatContext>
    auto format(_object_type_fancy type, FormatContext& ctx) const
    {
        return formatter<basic_string_view<ContextC>, ContextC>::format(type.fancy<ContextC>(), ctx);
    }
};

using _lazy_wstring = _chars_mixer<wchar_t, char>;

//---------

class dos_nt
{
    void construct(const LDR_DATA_TABLE_ENTRY* ldrEntry)
    {
        FD_ASSERT(ldrEntry);
        dos = static_cast<IMAGE_DOS_HEADER*>(ldrEntry->DllBase);
        // check for invalid DOS / DOS signature.
        FD_ASSERT(dos && dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
        nt = this->map<IMAGE_NT_HEADERS>(dos->e_lfanew);
        // check for invalid NT / NT signature.
        FD_ASSERT(nt && nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
    }

  public:
    // base address
    IMAGE_DOS_HEADER* dos;
    IMAGE_NT_HEADERS* nt;

    dos_nt(const LDR_DATA_TABLE_ENTRY* ldrEntry)
    {
        construct(ldrEntry);
    }

    dos_nt(const LDR_DATA_TABLE_ENTRY& ldrEntry)
    {
        construct(&ldrEntry);
    }

    PVOID base() const
    {
        return dos;
    }

    std::span<uint8_t> read() const
    {
        return { static_cast<uint8_t*>(base()), nt->OptionalHeader.SizeOfImage };
    }

    std::span<IMAGE_SECTION_HEADER> sections() const
    {
        return { IMAGE_FIRST_SECTION(nt), static_cast<size_t>(nt->FileHeader.NumberOfSections) };
    }

    template <typename T = uint8_t, typename Q>
    T* map(Q obj) const
    {
        const auto dosAddr = reinterpret_cast<uintptr_t>(dos);
        uintptr_t  offset;
        if constexpr (std::is_pointer_v<Q>)
            offset = reinterpret_cast<uintptr_t>(obj);
        else
            offset = static_cast<uintptr_t>(obj);
        return reinterpret_cast<T*>(dosAddr + offset);
    }
};

//---------

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
            const auto c = current_;
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
            const auto c = current_;
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

        bool operator==(const T* other) const
        {
            return CONTAINING_RECORD(current_, T, InMemoryOrderLinks) == other;
        }

        bool operator==(const iterator&) const = default;
    };

    win_list_view(const LIST_ENTRY* root = nullptr)
    {
        if (root)
        {
            root_ = root;
            return;
        }

#if defined(_M_X64) || defined(__x86_64__)
        const auto mem = NtCurrentTeb();
        FD_ASSERT(mem, "Teb not found");
        const auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
        const auto mem = reinterpret_cast<PEB*>(__readfsdword(0x30));
        FD_ASSERT(mem, "Peb not found");
        const auto ldr = mem->Ldr;
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
static bool operator==(const T* other, typename win_list_view<T, Deref>::iterator itr)
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

    fmt::wstring_view get() const
    {
        return get_std();
    }

    std::wstring_view get_std() const
    {
        const auto fullPath = _library_info_path(entry);
#if 0
    return std::wstring_view(file_view(fullPath).name());
#else
        const auto nameStart = fullPath.rfind('\\');
        FD_ASSERT(nameStart != fullPath.npos, "Unable to get the module name");
        return fullPath.substr(nameStart + 1);
#endif
    }

    operator fmt::wstring_view() const
    {
        return get_std();
    }
};

template <>
struct fmt::formatter<_library_info_name, wchar_t> : formatter<wstring_view, wchar_t>
{
};

static bool operator==(std::wstring_view left, _library_info_name right)
{
    return left == right.get();
}

// static std::wstring_view _found_or_not(const void* ptr)
//{
//     return ptr ? L"found" : L"not found";
// }
//
// static std::wstring_view _name_or_unknown(const LDR_DATA_TABLE_ENTRY* entry)
//{
//    return entry ? _library_info_name(entry) : L"unknown name";
//}
//
// static void _log_removed_entry(const std::wstring_view name, const LDR_DATA_TABLE_ENTRY* entry)
//{
//    spdlog::info( //-
//        L"{} -> removed! ({:#X})",
//        name,
//        reinterpret_cast<uintptr_t>(entry)
//    );
//}

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

static auto _log_found_object(_library_info_name entry, const std::wstring_view objectType, _lazy_wstring object, const void* addr)
{
    if (addr)
        spdlog::info(L"{} -> {} '{}' found! ({:p})", (entry), objectType, object, addr);
    else
        spdlog::warn(L"{} -> {} '{}' NOT found!", (entry), objectType, object);
}

static void _log_address_found(_library_info_name entry, _object_type_fancy rawName, const void* addr)
{
    if (addr)
        spdlog::info(L"{} -> {} found! ({:p})", (entry), (rawName), (addr));
    else
        spdlog::warn(L"{} -> {} NOT found!", (entry), (rawName));
}

static constexpr auto _HexDigits = "0123456789ABCDEF";

template <typename T>
[[nodiscard]]
static constexpr auto _bytes_to_sig(const T* bytes, const size_t size)
{
    const auto hexLength = /*(size << 1) + size*/ size * 3;

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
        const uint8_t currByte = (bytes)[n];

        // manually convert byte to chars
        (bytesOut)[i]     = _HexDigits[((currByte & 0xF0) >> 4)];
        (bytesOut)[i + 1] = _HexDigits[(currByte & 0x0F)];
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

class _demagle_type
{
    // fmt::basic_memory_buffer

    const char* type;

  public:
    _demagle_type(const char* typeDescriptor)
        : type(typeDescriptor)
    {
    }

    template <typename T>
    _chars_mixer<T, char> get() const
    {
        const auto prefix = type[std::size(_RttiInfo.rawPrefix)];
        if (prefix == _RttiInfo.classPrefix)
            return ("struct");
        if (prefix == _RttiInfo.structPrefix)
            return ("struct");

        return nullptr;
    }
};

template <typename T>
struct fmt::formatter<_demagle_type, T> : formatter<basic_string_view<T>, T>
{
    template <typename FormatCtx>
    auto format(_demagle_type type, FormatCtx& ctx) const
    {
        return formatter<basic_string_view<T>, T>::format(type.get<T>().native(), ctx);
    }
};

class _demagle_name
{
    std::string_view name;

  public:
    _demagle_name(const std::string_view name)
        : name(name)
    {
    }

    template <typename T>
    auto get() const
    {
        fmt::basic_memory_buffer<T> buff;
        if (name.contains('@'))
        {
            using boost::core::demangle;
            if (name.data()[name.size()] == '\0')
            {
                buff.append(demangle(name.data()));
            }
            else
            {
                if constexpr (std::same_as<T, char>)
                {
                    buff.append(name);
                    buff.push_back(0);
                    auto tmp = demangle(buff.data());
                    buff.resize(tmp.size());
                    buff.append(tmp);
                }
                else
                {
                    fmt::basic_memory_buffer<char> tmp;
                    tmp.append(name);
                    tmp.push_back(0);
                    buff.append(demangle(tmp.data()));
                }
            }
        }
        else if (auto pos = name.find(' '); pos != name.npos)
        {
            buff.append((name).substr(pos + 1));
        }
        else
        {
            buff.append(name);
        }

        return any_string_view(std::move(buff));
    }
};

template <typename T>
struct fmt::formatter<_demagle_name, T> : formatter<basic_string_view<T>, T>
{
    template <typename FormatCtx>
    auto format(_demagle_name name, FormatCtx& ctx) const
    {
        return formatter<basic_string_view<T>, T>::format(name.get<T>(), ctx);
    }
};

static void _log_found_vtable(_library_info_name entry, _demagle_type typeDescriptor, _demagle_name name, const void* vtablePtr)
{
    if (vtablePtr)
        spdlog::info(L"{} -> vtable for {} '{}' found! ({:p})", (entry), typeDescriptor, name, vtablePtr);
    else
        spdlog::warn(L"{} -> vtable for {} '{}' NOT found!", (entry), typeDescriptor, name);
}

namespace fd
{
static library_info _find_library(PVOID baseAddress)
{
    for (const auto* e : (ldr_tables_view_ptr()))
    {
        if (baseAddress == e->DllBase)
            return e;
    }
    return nullptr;
}

static library_info _find_library(std::wstring_view name)
{
    for (const auto* e : (ldr_tables_view_ptr()))
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
    return _library_info_name(entry_).get_std();
}

// void library_info::log_class_info(const std::string_view rawName, const void* addr) const
//{
//     _log_address_found(entry_, rawName, addr);
// }

class exports_view
{
    dos_nt dnt_;

    uint32_t* names_;
    uint32_t* funcs_;
    uint16_t* ords_;

    union
    {
        IMAGE_EXPORT_DIRECTORY* exportDirDir_;
        uint8_t*                virtualAddrStart_;
    };

    uint8_t* virtualAddrEnd_;

    using src_ptr = const exports_view*;

  public:
    exports_view(library_info::pointer entry)
        : dnt_(entry)
    {
        // get export data directory.
        const auto& dataDir = dnt_.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        FD_ASSERT(dataDir.VirtualAddress, "Current module doesn't have the virtual address!");

        // get export export_dir.
        exportDirDir_   = dnt_.map<IMAGE_EXPORT_DIRECTORY>(dataDir.VirtualAddress);
        virtualAddrEnd_ = virtualAddrStart_ + dataDir.Size;

        names_ = dnt_.map<uint32_t>(exportDirDir_->AddressOfNames);
        funcs_ = dnt_.map<uint32_t>(exportDirDir_->AddressOfFunctions);
        ords_  = dnt_.map<uint16_t>(exportDirDir_->AddressOfNameOrdinals);
    }

    const char* name(DWORD offset) const
    {
        return dnt_.map<const char>(names_[offset]);
    }

    void* function(DWORD offset) const
    {
        const auto tmp = dnt_.map<uint8_t>(funcs_[ords_[offset]]);
        if (tmp < virtualAddrStart_ || tmp >= virtualAddrEnd_)
            return tmp;

            // todo:resolve fwd export
            // FD_ASSERT_PANIC("Forwarded export detected");

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

        const char* name() const
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

        bool operator==(const iterator& other) const
        {
            if (offset_ == other.offset_)
            {
                FD_ASSERT(this->src_ == other.src_);
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

void* library_info::find_export(const std::string_view name) const
{
    void* exportPtr = nullptr;
    for (const auto val : exports_view(entry_))
    {
        if (val.name() == name)
        {
            exportPtr = val.function();
            break;
        }
    }

    _log_found_object(entry_, L"export", name, exportPtr);
    return exportPtr;
}

IMAGE_SECTION_HEADER* library_info::find_section(const std::string_view name) const
{
    IMAGE_SECTION_HEADER* header = nullptr;
    for (auto& h : dos_nt(entry_).sections())
    {
        if (reinterpret_cast<const char*>(h.Name) == name)
        {
            header = &h;
            break;
        }
    }

    _log_found_object(entry_, L"section", name, header);
    return header;
}

void* library_info::find_signature(const std::string_view sig) const
{
    const pattern_scanner finder(dos_nt(entry_).read());
    const auto            result = finder(sig).front();

    _log_found_object(entry_, L"signature", sig, result);
    return result;
}

enum class obj_type : uint8_t
{
    // ReSharper disable CppInconsistentNaming
    UNKNOWN,
    CLASS,
    STRUCT,
    NATIVE
    // ReSharper restore CppInconsistentNaming
};

class vtable_finder
{
    library_info info_;
    dos_nt       dnt_;

  public:
    vtable_finder(const library_info info)
        : info_(info)
        , dnt_(info.get())
    {
    }

    const char* find_type_descriptor(const std::string_view name, const obj_type type) const
    {
        const pattern_scanner_text wholeModuleFinder(dnt_.read());

        const void* rttiClassName;

        if (type == obj_type::UNKNOWN)
        {
            FD_ASSERT(!name.contains(' '));
            const auto bytesName = _bytes_to_sig(name.data(), name.size());

            fmt::memory_buffer realNameUnk;
            realNameUnk.append(_RttiInfo.rawPrefixBytes);
            realNameUnk.append(std::string_view(" ? "));
            realNameUnk.append(bytesName);
            realNameUnk.push_back(' ');
            realNameUnk.append(_RttiInfo.rawPostfixBytes);

            rttiClassName = wholeModuleFinder(realNameUnk).front();
        }
        else if (type == obj_type::NATIVE)
        {
            // const auto ptr  = (const uint8_t*)name.data();
            // FD_ASSERT(ptr >= whole_module_finder.from && ptr <= whole_module_finder.to - name.size(), "Selected wrong module!");
            rttiClassName = name.data();
        }
        else
        {
            char strPrefix;
            if (type == obj_type::CLASS)
                strPrefix = _RttiInfo.classPrefix;
            else if (type == obj_type::STRUCT)
                strPrefix = _RttiInfo.structPrefix;
            else
                // FD_ASSERT_PANIC("Unknown type");
                ;

            fmt::memory_buffer realName;
            realName.append(_RttiInfo.rawPrefix);
            realName.push_back(strPrefix);
            realName.append(name);
            realName.append(_RttiInfo.rawPostfix);

            rttiClassName = wholeModuleFinder.raw()(realName).front();
        }

        return static_cast<const char*>(rttiClassName);
    }

    void* operator()(const void* rttiClassName) const
    {
        // get rtti type descriptor
        auto typeDescriptor = reinterpret_cast<uintptr_t>(rttiClassName);
        // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
        typeDescriptor -= sizeof(uintptr_t) * 2;

        const auto dotRdata = info_.find_section(".rdata");
        const auto dotText  = info_.find_section(".text");

        const xrefs_scanner dotRdataFinder(dnt_.map(dotRdata->VirtualAddress), dotRdata->SizeOfRawData);
        const xrefs_scanner dotTextFinder(dnt_.map(dotText->VirtualAddress), dotText->SizeOfRawData);

        for (const auto xref : dotRdataFinder(typeDescriptor))
        {
            const auto val          = reinterpret_cast<uint32_t>(xref);
            // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
            const auto vtableOffset = *reinterpret_cast<uint32_t*>(val - 0x8);
            if (vtableOffset != 0)
                continue;

            const auto objectLocator = val - 0xC;
            const auto vtableAddress = reinterpret_cast<uintptr_t>(dotRdataFinder(objectLocator).front()) + 0x4;

            // check is valid offset
            FD_ASSERT(vtableAddress > sizeof(uintptr_t));
            return dotTextFinder(vtableAddress).front();
        }
        return nullptr;
    }
};

static void* _find_vtable(const library_info info, const std::string_view name, const obj_type type)
{
    const vtable_finder vtableFinder(info);

    const auto rttiClassName = vtableFinder.find_type_descriptor(name, type);
    FD_ASSERT(rttiClassName != nullptr);
    const auto vtablePtr = vtableFinder(rttiClassName);

    _log_found_vtable(info.get(), rttiClassName, name, vtablePtr);
    return vtablePtr;
}

void* library_info::find_vtable_class(const std::string_view name) const
{
    return _find_vtable(entry_, name, obj_type::CLASS);
}

void* library_info::find_vtable_struct(const std::string_view name) const
{
    return _find_vtable(entry_, name, obj_type::STRUCT);
}

void* library_info::find_vtable_unknown(const std::string_view name) const
{
    return _find_vtable(entry_, name, obj_type::UNKNOWN);
}

void* library_info::find_vtable(const std::string_view name) const
{
    void*      result  = nullptr;
    const auto tryFind = [&](const std::string_view type, const obj_type objType) -> bool {
        if (name.starts_with(type))
            result = _find_vtable(entry_, name.substr(type.size()), objType);
        return result;
    };

    if (tryFind("class ", obj_type::CLASS) || tryFind("struct ", obj_type::STRUCT))
        return result;

    return _find_vtable(entry_, name, obj_type::UNKNOWN);
}

void* library_info::find_vtable(const std::type_info& info) const
{
    return _find_vtable(entry_, info.raw_name(), obj_type::NATIVE);
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

#undef ERROR

class interface_reg
{
    void* (*createFn_)();
    const char*          name_;
    const interface_reg* next_;

  public:
    class iterator
    {
        const interface_reg* current_;

      public:
        constexpr iterator(const interface_reg* reg)
            : current_(reg)
        {
        }

        iterator& operator++()
        {
            current_ = current_->next_;
            return *this;
        }

        iterator operator++(int)
        {
            const auto tmp = current_;
            current_       = current_->next_;
            return tmp;
        }

        const interface_reg& operator*() const
        {
            return *current_;
        }

        bool operator==(const iterator&) const = default;
    };

    class range
    {
        const interface_reg* root_;

      public:
        range(const void* createInterfaceFn)
        {
            const auto relativeFn   = reinterpret_cast<uintptr_t>(createInterfaceFn) + 0x5;
            const auto displacement = *reinterpret_cast<int32_t*>(relativeFn);
            const auto jmp          = relativeFn + sizeof(int32_t) + displacement;

            root_ = **reinterpret_cast<interface_reg***>(jmp + 0x6);
        }

        range(const interface_reg* current)
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

    struct equal_t
    {
        enum value_type : uint8_t
        {
            // ReSharper disable CppInconsistentNaming
            FULL,
            PARTIAL,
            ERROR
            // ReSharper restore CppInconsistentNaming
        };

      private:
        value_type result_;

      public:
        equal_t(const value_type result)
            : result_(result)
        {
        }

        operator value_type() const
        {
            return result_;
        }

        explicit operator bool() const
        {
            return result_ != ERROR;
        }

        bool operator==(const value_type val) const
        {
            return result_ == val;
        }
    };

    interface_reg()                     = delete;
    interface_reg(const interface_reg&) = delete;

    equal_t operator==(const std::string_view ifcName) const
    {
        const std::string_view name = this->name_;
        if (ifcName.starts_with(name))
        {
            if (ifcName.size() == name.size()) // PARTIALly comared
                return equal_t::FULL;
            if (std::isdigit(name[ifcName.size()])) // PARTIAL name must be looks like IfcName001
                return equal_t::PARTIAL;
        }
        return equal_t::ERROR;
    }

    auto operator()() const
    {
        return (createFn_());
    }

    auto name_size(const std::string_view knownPart = {}) const
    {
#ifdef _DEBUG
        if (!knownPart.empty() && !knownPart.starts_with(this->name_))
            FD_ASSERT("Incorrect known part");
#endif

        const auto idxStart = this->name_ + knownPart.size();
        const auto idxSize  = std::strlen(idxStart);
        return knownPart.size() + idxSize;
    }

    auto name() const
    {
        return name_;
    }

    auto operator+(size_t offset) const
    {
        auto src = this;
        do
            src = src->next_;
        while (--offset != 0u);
        return src;
    }
};

csgo_library_info::csgo_library_info(library_info info)
    : library_info(info)
{
}

void* csgo_library_info::find_interface(const std::string_view name) const
{
    return find_interface(find_export("CreateInterface"), name);
}

void* csgo_library_info::find_interface(const void* createInterfaceFn, const std::string_view name) const
{
    FD_ASSERT(createInterfaceFn != nullptr);

    std::string_view logName;
    void*            ifcAddr = nullptr;

    for (auto& reg : interface_reg::range(createInterfaceFn))
    {
        const auto result = reg == name;
        if (!result)
            continue;

#ifndef _DEBUG

#endif
        if (result == interface_reg::equal_t::PARTIAL)
        {
            const auto wholeNameSize = reg.name_size(name);
#ifdef _DEBUG
            if (!std::all_of(reg.name() + name.size(), reg.name() + wholeNameSize, ::isdigit))
                FD_ASSERT("Incorrect given interface name");
            const auto nextReg = reg + 1;
            if (nextReg)
            {
                for (auto& reg1 : interface_reg::range(nextReg))
                {
                    if (reg1 == name)
                        FD_ASSERT("Duplicate interface name detected");
                }
            }

#endif
            logName = { reg.name(), wholeNameSize };
        }

        ifcAddr = (reg());
        break;
    }

    _log_found_object(this->get(), L"csgo interface", logName.empty() ? name : logName, ifcAddr);

    return ifcAddr;
}

static library_info _Current = nullptr;

static DECLSPEC_NOINLINE PVOID _self_module_handle()
{
    MEMORY_BASIC_INFORMATION    info;
    constexpr SIZE_T            infoSize = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] const auto len      = VirtualQueryEx(GetCurrentProcess(), _self_module_handle, &info, infoSize);
    FD_ASSERT(len == infoSize, "Wrong size");
    return static_cast<HINSTANCE>(info.AllocationBase);
}

static void _set_current_library()
{
    _Current = _find_library(_self_module_handle());
}

static void _set_current_library(HMODULE handle)
{
    FD_ASSERT(handle == _self_module_handle());
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
    for (auto& info : reversed(delayed_))
        info.sem.release();
}

library_info_cache::~library_info_cache()
{
    if (!cookie_)
        return;

    if (!NT_SUCCESS(notif_->unreg(cookie_)))
        // FD_ASSERT_PANIC("Unable to remove dll notification");
        ;

    if (!delayed_.empty())
    {
        cache_.clear();
        release_delayed();
    }
}

static void CALLBACK _on_new_library(const ULONG notificationReason, const PCLDR_DLL_NOTIFICATION_DATA notificationData, const PVOID context)
{
    const auto data = static_cast<library_info_cache*>(context);

    switch (notificationReason)
    {
    case LDR_DLL_NOTIFICATION_REASON_LOADED: {
        data->store(notificationData->Loaded.DllBase, _to_string_view(*notificationData->Loaded.BaseDllName));
        break;
    }
    case LDR_DLL_NOTIFICATION_REASON_UNLOADED: {
        data->remove(notificationData->Unloaded.DllBase, _to_string(*notificationData->Unloaded.BaseDllName));
        break;
    }
    default:
        // FD_ASSERT_PANIC("Unknown dll notification type");
        ;
    }

    // #if 0
    //         const auto target_name = _To_string_view(*NotificationData->Loaded.FullDllName);
    //         if (!target_name.ends_with(data->name))
    //             return;
    // #else
    //         if (_to_string_view(*notificationData->Loaded.BaseDllName) != data->name)
    //             return;
    // #endif
}

static std::mutex _LibraryInfoCacheIntMtx;

library_info_cache::library_info_cache()
{
    if (!notif_)
    {
        const std::lock_guard g(_LibraryInfoCacheIntMtx);
        if (!notif_)
        {
            const auto ntdll = _find_library(L"ntdll.dll");
            FD_ASSERT(ntdll != nullptr);
            const auto reg   = reinterpret_cast<LdrRegisterDllNotification>(ntdll.find_export("LdrRegisterDllNotification"));
            const auto unreg = reinterpret_cast<LdrUnregisterDllNotification>(ntdll.find_export("LdrUnregisterDllNotification"));

            auto& n = notif_.emplace();
            n.reg   = reg;
            n.unreg = unreg;
        }
    }

    const std::lock_guard g(mtx_);

    if (!NT_SUCCESS(notif_->reg(0, _on_new_library, this, &cookie_)))
        // FD_ASSERT_PANIC("Unable to create dll notification");
        ;

    for (auto entry : ldr_tables_view_ptr())
        cache_.emplace_back(entry);
}

void library_info_cache::store(PVOID baseAddress, std::wstring_view name)
{
    const std::lock_guard g(mtx_);
    FD_ASSERT(cookie_ != nullptr);

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
    const std::lock_guard g(mtx_);
    FD_ASSERT(cookie_ != nullptr);

    auto& last = cache_.back();
    auto  info = std::find(cache_.begin(), cache_.end() - 1, baseAddress);
    using std::swap;
    swap(*info, last);
    FD_ASSERT(last == baseAddress);
    cache_.pop_back();
}

static auto _try_get_from_cache(auto& cache, auto val)
{
    library_info result;
    for (auto& info : (cache))
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
    const std::lock_guard g(mtx_);
    FD_ASSERT(cookie_ != nullptr);
    return _try_get_from_cache(cache_, baseAddress);
}

library_info library_info_cache::get(std::wstring_view name) const
{
    const std::lock_guard g(mtx_);
    FD_ASSERT(cookie_ != nullptr);
    return _try_get_from_cache(cache_, name);
}

static bool operator==(const _delayed_library_info& info, const std::wstring_view name)
{
    return info.name == name;
}

library_info library_info_cache::get(std::wstring_view name)
{
    std::binary_semaphore* sem;
    {
        const std::lock_guard g(mtx_);
        FD_ASSERT(cookie_ != nullptr);

        auto cached = _try_get_from_cache(cache_, name);
        if (cached != nullptr)
            return cached;

        auto info = std::find(delayed_.begin(), delayed_.end(), name);
        if (info == delayed_.end())
            sem = &delayed_.emplace_back((name)).sem;
        else
            sem = &info->sem;
    }

    sem->acquire();
    return _try_get_from_cache(cache_, name);
}

void library_info_cache::destroy()
{
    const std::lock_guard g(mtx_);
    FD_ASSERT(cookie_ != nullptr);

    if (!NT_SUCCESS(notif_->unreg(cookie_)))
        // FD_ASSERT_PANIC("Unable to remove dll notification");
        ;

    cache_.clear();
    release_delayed();
    delayed_.clear();
    cookie_ = nullptr;
}
} // namespace fd