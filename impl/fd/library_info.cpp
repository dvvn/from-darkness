// ReSharper disable CppClangTidyClangDiagnosticMicrosoftCast
#include <fd/format.h>

#include <fd/assert.h>
#include <fd/filesystem.h>
#include <fd/functional.h>
#include <fd/library_info.h>
#include <fd/logger.h>
#include <fd/mem_scanner.h>
#include <fd/string_info.h>
#include <fd/views.h>

#include "demangle_symbol.h"
#include "dll_notification.h"

#include <semaphore>

namespace fd
{
class dos_nt
{
    void construct(const LDR_DATA_TABLE_ENTRY* ldrEntry)
    {
        FD_ASSERT(ldrEntry != nullptr);
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

    explicit dos_nt(const library_info info)
    {
        construct(info.get());
    }

    PVOID base() const
    {
        return dos;
    }

    range_view<uint8_t*> read() const
    {
        return { static_cast<uint8_t*>(base()), nt->OptionalHeader.SizeOfImage };
    }

    range_view<IMAGE_SECTION_HEADER*> sections() const
    {
        return { IMAGE_FIRST_SECTION(nt), nt->FileHeader.NumberOfSections };
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

template <typename T>
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

        T& operator*() const
        {
            return *CONTAINING_RECORD(current_, T, InMemoryOrderLinks);
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

    win_list_view(LIST_ENTRY* root = nullptr)
    {
        if (root)
        {
            root_ = root;
            return;
        }

#if defined(_M_X64) || defined(__x86_64__)
        const auto mem = NtCurrentTeb();
        FD_ASSERT(mem != nullptr, "Teb not found");
        const auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
        const auto mem = reinterpret_cast<PEB*>(__readfsdword(0x30));
        FD_ASSERT(mem != nullptr, "Peb not found");
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

template <typename T>
static bool operator==(const T* other, typename win_list_view<T>::iterator itr)
{
    return itr.operator==(other);
}

using ldr_tables_view = win_list_view<LDR_DATA_TABLE_ENTRY>;

static wstring_view _to_string_view(const UNICODE_STRING& ustr)
{
    return { ustr.Buffer, ustr.Length / sizeof(WCHAR) };
}

static wstring_view _library_info_path(const LDR_DATA_TABLE_ENTRY* entry)
{
    return _to_string_view(entry->FullDllName);
}

static wstring_view _library_info_name(const LDR_DATA_TABLE_ENTRY* entry)
{
    const auto fullPath = _library_info_path(entry);
#if 1
    return fs::basic_path(fullPath).filename();
#else
    const auto name_start = fullPath.rfind('\\');
    FD_ASSERT(name_start != fullPath.npos, "Unable to get the module name");
    return fullPath.substr(name_start + 1);
#endif
}

static wstring_view _found_or_not(const void* ptr)
{
    return ptr != nullptr ? L"found" : L"not found";
}

static wstring_view _name_or_unknown(const LDR_DATA_TABLE_ENTRY* entry)
{
    return entry ? _library_info_name(entry) : L"unknown name";
}

static void _log_found_entry(const wstring_view name, const LDR_DATA_TABLE_ENTRY* entry)
{
    if (!log_active())
        return;
    log_unsafe(format( //-
        L"{} -> {}! ({:#X})"sv,
        name,
        _found_or_not(entry),
        reinterpret_cast<uintptr_t>(entry)
    ));
}

static void _log_found_entry(const /*IMAGE_DOS_HEADER*/ void* baseAddress, const LDR_DATA_TABLE_ENTRY* entry)
{
    if (!log_active())
        return;
    log_unsafe(format( //-
        L"{:#X} ({}) -> {}! ({:#X})",
        reinterpret_cast<uintptr_t>(baseAddress),
        _name_or_unknown(entry),
        _found_or_not(entry),
        reinterpret_cast<uintptr_t>(entry)
    ));
}

template <typename T>
static auto _as_wstring(const T& str)
{
    if constexpr (std::same_as<std::iter_value_t<T>, wchar_t>)
        return str;
    else
    {
        auto tmp = forward_view_lazy(str);
        return wstring(tmp.begin(), tmp.end() - !std::is_class_v<T>);
    }
}

static auto _log_found_object(const LDR_DATA_TABLE_ENTRY* entry, const auto objectType, const auto object, const void* addr)
{
    if (!log_active())
        return;
    log_unsafe(format( //-
        L"{} -> {} '{}' {}! ({:#X})",
        _library_info_name(entry),
        _as_wstring(objectType), // wstring conversion suck, find better way
        _as_wstring(object),
        _found_or_not(addr),
        reinterpret_cast<uintptr_t>(addr)
    ));
}

static void _log_address_found(const LDR_DATA_TABLE_ENTRY* entry, const string_view rawName, const void* addr)
{
    if (!log_active())
        return;

    const auto nameBegin = rawName.find(' '); // class or struct
    if (nameBegin == rawName.npos)
    {
        _log_found_object(entry, L"object", rawName, addr);
    }
    else
    {
        const auto objectType = rawName.substr(0, nameBegin);
        FD_ASSERT(!objectType.empty(), "Wrong object type");
        const auto object = rawName.substr(nameBegin + 1);
        FD_ASSERT(!object.empty(), "Wrong object name");
        _log_found_object(entry, objectType, object, addr);
    }
}

static constexpr auto _HexDigits = "0123456789ABCDEF";

template <typename T>
[[nodiscard]] static constexpr auto _bytes_to_sig(const T* bytes, const size_t size)
{
    const auto hexLength = /*(size << 1) + size*/ size * 3;

    // construct pre-reserved string filled with spaces
    string pattern(hexLength - 1, ' ');

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
    // construct pre-reserved string filled with spaces
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
    char rawPrefix[3]  = { '.', '?', 'A' };
    char rawPostfix[2] = { '@', '@' };

#define SPACES1 ' '
#define SPACES2 ' ', ' '
#define SPACES3 SPACES2, ' '

    uint8_t rawPrefixBytes[3 * 3 - 1]{ SPACES3, SPACES3, SPACES2 };
    uint8_t rawPostfixBytes[2 * 3 - 1]{ SPACES2, SPACES2, SPACES1 };

#undef SPACES1
#undef SPACES2
#undef SPACES3

    char classPrefix  = 'V';
    char structPrefix = 'U';

  private:
    [[no_unique_address]] std::false_type dummy1_ = _bytes_to_sig(rawPrefixBytes, rawPrefix, 3 * 3);
    [[no_unique_address]] std::false_type dummy2_ = _bytes_to_sig(rawPostfixBytes, rawPostfix, 2 * 3);
} _RttiInfo;

static void _log_found_vtable(const LDR_DATA_TABLE_ENTRY* entry, const string_view name, const char* typeDescriptor, const void* vtablePtr)
{
#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

    const auto demagleType = [=] {
        const auto prefix = typeDescriptor[std::size(_RttiInfo.rawPrefix)];
        if (prefix == _RttiInfo.classPrefix)
            return L"class";
        if (prefix == _RttiInfo.structPrefix)
            return L"struct";
        FD_ASSERT_PANIC("Unknown prefix!");
    };

    const auto demagleName = [=]() -> wstring {
        if (name.contains('@'))
        {
#ifdef _DEBUG
            DebugBreak(); // look at buff
#endif
            const auto buff = demangle_symbol(typeDescriptor);
            return { buff.begin() + buff.find(' ') + 1, buff.end() };
        }
        if (name.contains(' '))
        {
            return { name.begin() + name.find(' ') + 1, name.end() };
        }
        return { name.begin(), name.end() };
    };

    if (!log_active())
        return;
    log_unsafe(format( //-
        L"{} -> {} {} '{}' {}! ({:#X})",
        _library_info_name(entry),
        L"vtable for",
        demagleType(),
        demagleName(),
        _found_or_not(vtablePtr),
        reinterpret_cast<uintptr_t>(vtablePtr)
    ));
}

static library_info _find_library(PVOID baseAddress, bool notify, wstring_view name = {})
{
    library_info::pointer entry = nullptr;
    for (auto& e : ldr_tables_view())
    {
        if (baseAddress == dos_nt(e).base())
        {
            entry = &e;
            break;
        }
    }

    if (notify)
    {
        if (name.empty())
            _log_found_entry(baseAddress, entry);
        else
            _log_found_entry(name, entry);
    }
    return entry;
}

library_info find_library(PVOID baseAddress, bool notify)
{
    return _find_library(baseAddress, notify);
}

library_info find_library(wstring_view name, bool notify)
{
    library_info::pointer entry = nullptr;
    for (auto& e : ldr_tables_view())
    {
        if (_library_info_name(&e) == name)
        {
            entry = &e;
            break;
        }
    }
    if (notify /*&& entry*/)
        _log_found_entry(name, entry);
    return entry;
}

struct callback_data_t
{
    wstring_view          name;
    std::binary_semaphore sem;
    PVOID                 found;

    callback_data_t(const wstring_view name)
        : name(name)
        , sem(1)
        , found(nullptr)
    {
    }
};

static void CALLBACK _on_new_library(const ULONG notificationReason, const PCLDR_DLL_NOTIFICATION_DATA notificationData, const PVOID context)
{
    const auto data = static_cast<callback_data_t*>(context);

    if (notificationReason == LDR_DLL_NOTIFICATION_REASON_UNLOADED)
    {
        // if (notificationData->Unloaded.DllBase != current_library_info(false)->DllBase)
        return;
    }
    else // NOLINT(readability-else-after-return)
    {
#if 0
        const auto target_name = _To_string_view(*NotificationData->Loaded.FullDllName);
        if (!target_name.ends_with(data->name))
            return;
#else
        if (_to_string_view(*notificationData->Loaded.BaseDllName) != data->name)
            return;
#endif
        data->found = notificationData->Loaded.DllBase;
    }

    data->sem.release();
}

static auto _wait_prepare(const bool notify)
{
    const auto ntdll = find_library(L"ntdll.dll", notify);

    const auto reg   = reinterpret_cast<LdrRegisterDllNotification>(ntdll.find_export("LdrRegisterDllNotification"));
    const auto unreg = reinterpret_cast<LdrUnregisterDllNotification>(ntdll.find_export("LdrUnregisterDllNotification"));

    return std::pair(reg, unreg);
}

library_info wait_for_library(const wstring_view name, bool notify)
{
    static const auto [reg_fn, unreg_fn] = _wait_prepare(notify);

    callback_data_t cbData(name);
    void*           cookie;
    if (!NT_SUCCESS(reg_fn(0, _on_new_library, &cbData, &cookie)))
        return nullptr;
    cbData.sem.acquire();
    if (!NT_SUCCESS(unreg_fn(cookie)))
        return nullptr;
    if (!cbData.found)
        return nullptr;
    return _find_library(cbData.found, notify, name);
}

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

library_info::operator bool() const
{
    return static_cast<bool>(entry_);
}

wstring_view library_info::path() const
{
    return _library_info_path(entry_);
}

wstring_view library_info::name() const
{
    return _library_info_name(entry_);
}

void library_info::log_class_info(const string_view rawName, const void* addr) const
{
    _log_address_found(entry_, rawName, addr);
}

class exports_view
{
    dos_nt dnt_;

    uint32_t* names_;
    uint32_t* funcs_;
    uint16_t* ords_;

    union
    {
        IMAGE_EXPORT_DIRECTORY* export_dir_;
        uint8_t*                virtual_addr_start_;
    };

    uint8_t* virtual_addr_end_;

    using src_ptr = const exports_view*;

  public:
    exports_view(library_info::pointer entry)
        : dnt_(entry)
    {
        // get export data directory.
        const auto& dataDir = dnt_.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        FD_ASSERT(dataDir.VirtualAddress, "Current module doesn't have the virtual address!");

        // get export export_dir.
        export_dir_       = dnt_.map<IMAGE_EXPORT_DIRECTORY>(dataDir.VirtualAddress);
        virtual_addr_end_ = virtual_addr_start_ + dataDir.Size;

        names_ = dnt_.map<uint32_t>(export_dir_->AddressOfNames);
        funcs_ = dnt_.map<uint32_t>(export_dir_->AddressOfFunctions);
        ords_  = dnt_.map<uint16_t>(export_dir_->AddressOfNameOrdinals);
    }

    const char* name(DWORD offset) const
    {
        return dnt_.map<const char>(names_[offset]);
    }

    void* function(DWORD offset) const
    {
        const auto tmp = dnt_.map<uint8_t>(funcs_[ords_[offset]]);
        if (tmp < virtual_addr_start_ || tmp >= virtual_addr_end_)
            return tmp;

        // todo:resolve fwd export
        FD_ASSERT_PANIC("Forwarded export detected");

#if 0
		// get forwarder string.
		const string_view fwd_str = export_ptr.get<const char*>( );

		// forwarders have a period as the delimiter.
		const auto delim = fwd_str.find_last_of('.');
		if(delim == fwd_str.npos)
			continue;

		using namespace string_view_literals;
		// get forwarder mod name.
		const info_string::fixed_type fwd_module_str = nstd::append<wstring>(fwd_str.substr(0, delim), L".dll"sv);

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
        return { this, std::min(export_dir_->NumberOfNames, export_dir_->NumberOfFunctions) };
    }
};

void* library_info::find_export(const string_view name, const bool notify) const
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

    if (notify)
        _log_found_object(entry_, L"export", name, exportPtr);

    return exportPtr;
}

IMAGE_SECTION_HEADER* library_info::find_section(const string_view name, const bool notify) const
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
    if (notify)
        _log_found_object(entry_, L"section", name, header);
    return header;
}

void* library_info::find_signature(const string_view sig, const bool notify) const
{
    const auto            memorySpan = dos_nt(entry_).read();
    const pattern_scanner finder(memorySpan.begin(), memorySpan.size());
    const auto            result = *finder(sig).begin();
    if (notify)
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
        , dnt_(info)
    {
    }

    const char* find_type_descriptor(const string_view name, const obj_type type) const
    {
        const auto                 memorySpan = dnt_.read();
        const pattern_scanner_text wholeModuleFinder(memorySpan.begin(), memorySpan.size());

        const void* rttiClassName;

        if (type == obj_type::UNKNOWN)
        {
            const auto           bytesName = _bytes_to_sig(name.data(), name.size());
            std::vector<uint8_t> realNameUnk; // no sting because no SSO anyway
            write_string(realNameUnk, _RttiInfo.rawPrefixBytes, " ? ", bytesName, ' ', _RttiInfo.rawPostfixBytes);

            rttiClassName = *wholeModuleFinder(realNameUnk.data(), realNameUnk.size()).begin();
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
                FD_ASSERT_PANIC("Unknown type");

            const auto realName = make_string(_RttiInfo.rawPrefix, strPrefix, name, _RttiInfo.rawPostfix);
            rttiClassName       = *wholeModuleFinder.raw()(realName).begin();
        }

        return static_cast<const char*>(rttiClassName);
    }

    void* operator()(const void* rttiClassName) const
    {
        // get rtti type descriptor
        auto typeDescriptor = reinterpret_cast<uintptr_t>(rttiClassName);
        // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
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
            const auto vtableAddress = reinterpret_cast<uintptr_t>(*dotRdataFinder(objectLocator).begin()) + 0x4;

            // check is valid offset
            FD_ASSERT(vtableAddress > sizeof(uintptr_t));
            return *dotTextFinder(vtableAddress).begin();
        }
        return nullptr;
    }
};

static void* _find_vtable(const library_info info, const string_view name, const obj_type type, const bool notify)
{
    const vtable_finder vtableFinder(info);

    const auto rttiClassName = vtableFinder.find_type_descriptor(name, type);
    FD_ASSERT(rttiClassName != nullptr);
    const auto vtablePtr = vtableFinder(rttiClassName);
    if (notify)
        _log_found_vtable(info.get(), name, rttiClassName, vtablePtr);

    return vtablePtr;
}

void* library_info::find_vtable_class(const string_view name, const bool notify) const
{
    return _find_vtable(entry_, name, obj_type::CLASS, notify);
}

void* library_info::find_vtable_struct(const string_view name, const bool notify) const
{
    return _find_vtable(entry_, name, obj_type::STRUCT, notify);
}

void* library_info::find_vtable_unknown(const string_view name, const bool notify) const
{
    return _find_vtable(entry_, name, obj_type::UNKNOWN, notify);
}

void* library_info::find_vtable(const string_view name, const bool notify) const
{
    constexpr string_view classP("class ");
    if (name.starts_with(classP))
        return _find_vtable(entry_, name.substr(classP.size()), obj_type::CLASS, notify);

    constexpr string_view structP("struct ");
    if (name.starts_with(structP))
        return _find_vtable(entry_, name.substr(structP.size()), obj_type::CLASS, notify);

    return _find_vtable(entry_, name, obj_type::UNKNOWN, notify);
}

void* library_info::find_vtable(const std::type_info& info, const bool notify) const
{
    const auto rawName = info.raw_name();
    return _find_vtable(entry_, { rawName, notify ? str_len(rawName) : 0 }, obj_type::NATIVE, notify);
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

    equal_t operator==(const string_view ifcName) const
    {
        if (equal(ifcName, this->name_))
        {
            const auto lastChar = this->name_[ifcName.size()];
            if (lastChar == '\0') // partially comared
                return equal_t::FULL;
            if (is_digit(lastChar)) // partial name must be looks like IfcName001
                return equal_t::PARTIAL;
        }
        return equal_t::ERROR;
    }

    auto operator()() const
    {
        return (createFn_());
    }

    auto name_size(const string_view knownPart = {}) const
    {
#ifdef _DEBUG
        if (!knownPart.empty() && !equal(knownPart, this->name_))
            FD_ASSERT("Incorrect known part");
#endif

        const auto idxStart = this->name_ + knownPart.size();
        const auto idxSize  = str_len(idxStart);
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

void* csgo_library_info::find_interface(const string_view name, const bool notify) const
{
    return find_interface(find_export("CreateInterface"), name, notify);
}

void* csgo_library_info::find_interface(const void* createInterfaceFn, const string_view name, const bool notify) const
{
    if (createInterfaceFn == nullptr)
        return nullptr;

    string_view logName;
    void*       ifcAddr = nullptr;

    for (auto& reg : interface_reg::range(createInterfaceFn))
    {
        const auto result = reg == name;
        if (!result)
            continue;

#ifndef _DEBUG
        if (notify)
#endif
            if (result == interface_reg::equal_t::PARTIAL)
            {
                const auto wholeNameSize = reg.name_size(name);
#ifdef _DEBUG
                if (!std::all_of(reg.name() + name.size(), reg.name() + wholeNameSize, is_digit))
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
                if (notify)
#endif
                    logName = { reg.name(), wholeNameSize };
            }

        ifcAddr = (reg());
        break;
    }

    if (notify)
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

static void _set_current_library(bool notify)
{
    _Current = find_library(_self_module_handle(), notify);
}

static void _set_current_library(HMODULE handle, bool notify)
{
    FD_ASSERT(handle == _self_module_handle());
    _Current = find_library(handle, notify);
}

library_info current_library_info(const bool notify)
{
    if (!_Current)
        _set_current_library(notify);
    else if (notify)
        _log_found_entry(_Current.name(), _Current.get());

    return _Current;
}

void set_current_library(const HMODULE handle, bool notify)
{
    _set_current_library(handle, notify);
}
}