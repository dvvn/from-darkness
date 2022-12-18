// ReSharper disable CppClangTidyClangDiagnosticMicrosoftCast
#include <fd/assert.h>
#include <fd/filesystem.h>
#include <fd/functional.h>
#include <fd/library_info.h>
#include <fd/logger.h>
#include <fd/mem_scanner.h>
#include <fd/string_info.h>

#include "demangle_symbol.h"
#include "dll_notification.h"

#include <algorithm>
#include <array>
#include <semaphore>

using namespace fd;

void dos_nt::construct(const LDR_DATA_TABLE_ENTRY* ldrEntry)
{
    FD_ASSERT(ldrEntry != nullptr);
    dos = static_cast<IMAGE_DOS_HEADER*>(ldrEntry->DllBase);
    // check for invalid DOS / DOS signature.
    FD_ASSERT(dos && dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    nt = this->map<IMAGE_NT_HEADERS>(dos->e_lfanew);
    // check for invalid NT / NT signature.
    FD_ASSERT(nt && nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
}

dos_nt::dos_nt(const LDR_DATA_TABLE_ENTRY* ldrEntry)
{
    construct(ldrEntry);
}

dos_nt::dos_nt(const library_info info)
{
    construct(info.get());
}

PVOID dos_nt::base() const
{
    return dos;
}

std::span<uint8_t> dos_nt::read() const
{
    return { static_cast<uint8_t*>(base()), nt->OptionalHeader.SizeOfImage };
}

std::span<IMAGE_SECTION_HEADER> dos_nt::sections() const
{
    return { IMAGE_FIRST_SECTION(nt), nt->FileHeader.NumberOfSections };
}

//---------

// ReSharper disable once CppInconsistentNaming
class LIST_ENTRY_iterator
{
    LIST_ENTRY* current_;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = size_t;
    using value_type = LIST_ENTRY;
    using pointer = value_type*;
    using reference = value_type&;

    LIST_ENTRY_iterator(LIST_ENTRY* current)
        : current_(current)
    {
    }

    LIST_ENTRY_iterator& operator++()
    {
        current_ = current_->Flink;
        return *this;
    }

    LIST_ENTRY_iterator operator++(int)
    {
        const auto tmp = *this;
        operator++();
        return tmp;
    }

    LIST_ENTRY_iterator& operator--()
    {
        current_ = current_->Blink;
        return *this;
    }

    LIST_ENTRY_iterator operator--(int)
    {
        const auto tmp = *this;
        operator--();
        return tmp;
    }

    reference operator*() const
    {
        return *current_;
    }

    pointer operator->() const
    {
        return current_;
    }

    bool operator==(const LIST_ENTRY_iterator& other) const = default;
};

// ReSharper disable once CppInconsistentNaming
class LIST_ENTRY_range
{
    LIST_ENTRY* root_;

  public:
    LIST_ENTRY_range()
    {
        const auto mem =
#if defined(_M_X64) || defined(__x86_64__)
            NtCurrentTeb();
        FD_ASSERT(mem != nullptr, "Teb not found");
        const auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
            reinterpret_cast<PEB*>(__readfsdword(0x30));
        FD_ASSERT(mem != nullptr, "Peb not found");
        const auto ldr = mem->Ldr;
#endif
        // get module linked list.
        root_ = &ldr->InMemoryOrderModuleList;
    }

    LIST_ENTRY_iterator begin() const
    {
        return root_->Flink;
    }

    LIST_ENTRY_iterator end() const
    {
        return root_;
    }
};

namespace std
{
    // ReSharper disable CppInconsistentNaming
    template <typename T>
    static T* get(LIST_ENTRY& list)
    {
        return CONTAINING_RECORD(&list, T, InMemoryOrderLinks);
    }

    template <typename T>
    static const T* get(const LIST_ENTRY& list)
    {
        return CONTAINING_RECORD(&list, T, InMemoryOrderLinks);
    }

    // ReSharper restore CppInconsistentNaming
} // namespace std

// ReSharper disable All

template <typename T, typename Fn>
static T* LIST_ENTRY_finder(Fn fn)
{
    for (auto& list : LIST_ENTRY_range())
    {
        auto item = std::get<T>(list);
        if (invoke(fn, item))
            return item;
    }
    return nullptr;
}

template <typename Fn>
static auto LDR_ENTRY_finder(Fn fn)
{
    return LIST_ENTRY_finder<LDR_DATA_TABLE_ENTRY>(fn);
}

// ReSharper restore All

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

static void _log_found_entry(const wstring_view name, const LDR_DATA_TABLE_ENTRY* entry)
{
    invoke( //
        Logger,
        L"{} -> {}! ({:#X})",
        name,
        bind_front(_found_or_not, entry),
        reinterpret_cast<uintptr_t>(entry)
    );
}

static void _log_found_entry(const IMAGE_DOS_HEADER* baseAddress, const LDR_DATA_TABLE_ENTRY* entry)
{
    const auto getName = [=] {
        return entry != nullptr ? _library_info_name(entry) : L"unknown name";
    };
    invoke( //
        Logger,
        L"{:#X} ({}) -> {}! ({:#X})",
        reinterpret_cast<uintptr_t>(baseAddress),
        getName,
        bind_front(_found_or_not, entry),
        reinterpret_cast<uintptr_t>(entry)
    );
}

// ReSharper disable once CppInconsistentNaming
static constexpr auto _log_found_object = [](const LDR_DATA_TABLE_ENTRY* entry, const auto objectType, const auto object, const void* addr) {
    invoke( //
        Logger,
        L"{} -> {} '{}' {}! ({:#X})",
        bind_front(_library_info_name, entry),
        objectType,
        object,
        bind_front(_found_or_not, addr),
        reinterpret_cast<uintptr_t>(addr)
    );
};

static void _log_address_found(const LDR_DATA_TABLE_ENTRY* entry, const string_view rawName, const void* addr)
{
    if (Logger == nullptr)
        return;

    // ReSharper disable once CppInconsistentNaming
    const auto _log_found_object_ex = [&](const auto objectType, const auto object) {
        _log_found_object(entry, objectType, object, addr);
    };

    const auto nameBegin = rawName.find(' '); // class or struct
    if (nameBegin == rawName.npos)
    {
        _log_found_object_ex(L"object", rawName);
    }
    else
    {
        const auto objectType = rawName.substr(0, nameBegin);
        FD_ASSERT(!objectType.empty(), "Wrong object type");
        const auto object = rawName.substr(nameBegin + 1);
        FD_ASSERT(!object.empty(), "Wrong object name");
        _log_found_object_ex(objectType, object);
    }
}

template <typename T>
static constexpr auto _bytes_to_sig(const T* bytes, const size_t size)
{
    constexpr auto hexDigits = "0123456789ABCDEF";
    const auto hexLength = /*(size << 1) + size*/ size * 3;

    // construct pre-reserved string filled with spaces
    string pattern(hexLength - 1, ' ');

    for (size_t i = 0, n = 0; i < hexLength; ++n, i += 3)
    {
        const uint8_t currByte = bytes[n];

        // manually convert byte to chars
        pattern[i] = hexDigits[((currByte & 0xF0) >> 4)];
        pattern[i + 1] = hexDigits[(currByte & 0x0F)];
    }

    return pattern;
}

template <size_t S>
using chars_array = std::array<char, S>;

template <size_t S>
static constexpr auto _bytes_to_sig(const chars_array<S>& str)
{
    const auto tmp = _bytes_to_sig(str.data(), str.size());
    chars_array<S * 3> buff{};
    std::ranges::copy(tmp, buff.begin());
    return buff;
}

static constexpr struct
{
    chars_array<3> rawPrefix = { '.', '?', 'A' };
    chars_array<2> rawPostfix = { '@', '@' };

    chars_array<3 * 3> rawPrefixBytes = _bytes_to_sig(rawPrefix);
    chars_array<2 * 3> rawPostfixBytes = _bytes_to_sig(rawPostfix);

    char classPrefix = 'V';
    char structPrefix = 'U';
} _RttiInfo;

static void _log_found_vtable(const LDR_DATA_TABLE_ENTRY* entry, const string_view name, const char* typeDescriptor, const void* vtablePtr)
{
#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

    const auto demagleType = [=]() -> string_view {
        const auto prefix = typeDescriptor[_RttiInfo.rawPrefix.size()];
        if (prefix == _RttiInfo.classPrefix)
            return "class";
        if (prefix == _RttiInfo.structPrefix)
            return "struct";
        FD_ASSERT_UNREACHABLE("Unknown prefix!");
    };

    const auto demagleName = [=]() -> string {
        string buff;
        if (name.contains('@'))
            buff = demangle_symbol(typeDescriptor);
        else if (name.contains(' '))
            buff = name;
        else
            return { name.data(), name.size() };

        return buff.substr(buff.find(' ') + 1);
    };

    invoke(
        Logger,
        L"{} -> {} {} '{}' {}! ({:#X})",
        bind_front(_library_info_name, entry),
        L"vtable for",
        demagleType,
        demagleName,
        bind_front(_found_or_not, vtablePtr),
        reinterpret_cast<uintptr_t>(vtablePtr)
    );
}

library_info fd::find_library(wstring_view name, bool notify)
{
    const auto entry = LDR_ENTRY_finder([=](const library_info info) {
        return info.name() == name;
    });
    if (notify /*&& entry*/)
        _log_found_entry(name, entry);
#ifdef _DEBUG
    if (!entry)
        return {}; // prevent assert
#endif
    return entry;
}

struct callback_data_t
{
    wstring_view name;
    std::binary_semaphore sem;
    PVOID found;

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
        if (notificationData->Unloaded.DllBase != current_library_info(false)->DllBase)
            return;
    }
    else
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
    const library_info ntdll(L"ntdll.dll", false, notify);

    const auto reg = reinterpret_cast<LdrRegisterDllNotification>(ntdll.find_export("LdrRegisterDllNotification"));
    const auto unreg = reinterpret_cast<LdrUnregisterDllNotification>(ntdll.find_export("LdrUnregisterDllNotification"));

    return std::pair(reg, unreg);
}

PVOID fd::wait_for_library(const wstring_view name)
{
    static const auto [reg_fn, unreg_fn] = _wait_prepare(false);

    callback_data_t cbData(name);
    void* cookie;
    if (!NT_SUCCESS(reg_fn(0, _on_new_library, &cbData, &cookie)))
        return nullptr;
    cbData.sem.acquire();
    if (!NT_SUCCESS(unreg_fn(cookie)))
        return nullptr;
    return cbData.found;
}

library_info::library_info()
    : entry_(nullptr)
{
}

library_info::library_info(const pointer entry)
    : entry_(entry)
{
    FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(const wstring_view name, const bool wait, const bool notify)
    : entry_(LDR_ENTRY_finder([=](const library_info info) {
        return info.name() == name;
    }))
{
    if (entry_ == nullptr && wait)
    {
        const auto baseAddress = wait_for_library(name);
        if (baseAddress != nullptr)
        {
            entry_ = LDR_ENTRY_finder([=](const dos_nt dnt) {
                return baseAddress == dnt.base();
            });
        }
    }

    if (notify)
        _log_found_entry(name, entry_);
    else
        FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(const IMAGE_DOS_HEADER* baseAddress, const bool notify)
    : entry_(LDR_ENTRY_finder([=](const dos_nt dnt) {
        return baseAddress == dnt.base();
    }))
{
    if (notify)
        _log_found_entry(baseAddress, entry_);
    else
        FD_ASSERT(entry_ != nullptr);
}

bool library_info::is_root() const
{
    /* bool first;
    LDR_ENTRY_finder([&](const LDR_DATA_TABLE_ENTRY* entry) {
        first = entry_ == entry;
        return true;
    });
    return first; */

    return entry_ == std::get<LDR_DATA_TABLE_ENTRY>(*LIST_ENTRY_range().begin());
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

void* library_info::find_export(const string_view name, const bool notify) const
{
    if (entry_ == nullptr)
        return nullptr;
    // base address
    const dos_nt dnt(entry_);

    // get export data directory.
    const auto& dataDir = dnt.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    FD_ASSERT(dataDir.VirtualAddress, "Current module doesn't have the virtual address!");

    // get export export_dir.
    const auto exportDir = dnt.map<const IMAGE_EXPORT_DIRECTORY>(dataDir.VirtualAddress);

    const auto exportDirMin = reinterpret_cast<const uint8_t*>(exportDir);
    const auto exportDirMax = exportDirMin + dataDir.Size;

    // names / funcs / ordinals ( all of these are RVAs ).
    const auto names = dnt.map<const uint32_t>(exportDir->AddressOfNames);
    const auto funcs = dnt.map<const uint32_t>(exportDir->AddressOfFunctions);
    const auto ords = dnt.map<const uint16_t>(exportDir->AddressOfNameOrdinals);

    void* exportPtr = nullptr;

    // iterate names array.
    for (size_t i = 0; i < exportDir->NumberOfNames; ++i)
    {
        const auto exportName = dnt.map<const char>(names[i]);
        if (exportName == nullptr)
            continue;
        if (exportName != name)
            continue;

        const auto exportPtrTemp = dnt.map<uint8_t>(funcs[ords[i]]);
        if (exportPtrTemp < exportDirMin || exportPtrTemp >= exportDirMax)
        {
            exportPtr = reinterpret_cast<void*>(exportPtrTemp);
            break;
        }

        FD_ASSERT_UNREACHABLE("Forwarded export detected");
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
    }

    if (notify)
        _log_found_object(entry_, L"export", name, exportPtr);

    return exportPtr;
}

IMAGE_SECTION_HEADER* library_info::find_section(const string_view name, const bool notify) const
{
    const auto sections = dos_nt(entry_).sections();
    const auto headerFound = std::ranges::find(sections.data(), sections.data() + sections.size(), name, [](auto& header) {
        return reinterpret_cast<const char*>(header.Name);
    });
    if (notify)
        _log_found_object(entry_, L"section", name, headerFound);
    return headerFound;
}

void* library_info::find_signature(const string_view sig, const bool notify) const
{
    const auto memorySpan = dos_nt(entry_).read();
    const pattern_scanner finder(memorySpan.data(), memorySpan.size());
    const auto result = *invoke(finder, sig);
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
    dos_nt dnt_;

  public:
    vtable_finder(const library_info info)
        : info_(info)
        , dnt_(info)
    {
    }

    const char* find_type_descriptor(const string_view name, const obj_type type) const
    {
        const auto memorySpan = dnt_.read();
        const pattern_scanner wholeModuleFinder(memorySpan.data(), memorySpan.size());

        const void* rttiClassName;

        if (type == obj_type::UNKNOWN)
        {
            const auto bytesName = _bytes_to_sig(name.data(), name.size());
            const auto realNameUnk = make_string(_RttiInfo.rawPrefixBytes, " ? ", bytesName, ' ', _RttiInfo.rawPostfixBytes);
            rttiClassName = *invoke(wholeModuleFinder, realNameUnk);
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
                FD_ASSERT_UNREACHABLE("Unknown type");

            const auto realName = make_string(_RttiInfo.rawPrefix, strPrefix, name, _RttiInfo.rawPostfix);
            rttiClassName = *invoke(wholeModuleFinder.raw(), realName);
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
        const auto dotText = info_.find_section(".text");

        const xrefs_scanner dotRdataFinder(dnt_.map(dotRdata->VirtualAddress), dotRdata->SizeOfRawData);
        const xrefs_scanner dotTextFinder(dnt_.map(dotText->VirtualAddress), dotText->SizeOfRawData);

        for (const auto xref : invoke(dotRdataFinder, typeDescriptor))
        {
            const auto val = reinterpret_cast<uint32_t>(xref);
            // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
            const auto vtableOffset = *reinterpret_cast<uint32_t*>(val - 0x8);
            if (vtableOffset != 0)
                continue;

            const auto objectLocator = val - 0xC;
            const auto vtableAddress = reinterpret_cast<uintptr_t>(*invoke(dotRdataFinder, objectLocator)) + 0x4;

            // check is valid offset
            FD_ASSERT(vtableAddress > sizeof(uintptr_t));
            return *invoke(dotTextFinder, vtableAddress);
        }
        return nullptr;
    }
};

static void* _find_vtable(const library_info info, const string_view name, const obj_type type, const bool notify)
{
    const vtable_finder vtableFinder(info);

    const auto rttiClassName = vtableFinder.find_type_descriptor(name, type);
    FD_ASSERT(rttiClassName != nullptr);
    const auto vtablePtr = invoke(vtableFinder, rttiClassName);
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
    /* auto do_find = bind_back(bind_front(_Find_vtable, entry_), notify);

     constexpr string_view class_prefix = "class ";
     if (name.starts_with(class_prefix))
         return do_find(name.substr(class_prefix.size()), obj_type::CLASS);

     constexpr string_view struct_prefix = "struct ";
     if (name.starts_with(struct_prefix))
         return do_find(name.substr(struct_prefix.size()), obj_type::STRUCT); */

    const rewrapped_namespaces helper(name);
    return _find_vtable(entry_, helper.name(), helper.is_class() ? obj_type::CLASS : helper.is_struct() ? obj_type::STRUCT : obj_type::UNKNOWN, notify);
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
    const char* name_;
    const interface_reg* next_;

  public:
    class iterator
    {
        const interface_reg* current_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = size_t;

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
            const auto tmp = *this;
            operator++();
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
            const auto relativeFn = reinterpret_cast<uintptr_t>(createInterfaceFn) + 0x5;
            const auto displacement = *reinterpret_cast<int32_t*>(relativeFn);
            const auto jmp = relativeFn + sizeof(int32_t) + displacement;

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

    struct equal
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
        equal(const value_type result)
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

    interface_reg() = delete;
    interface_reg(const interface_reg&) = delete;

    equal operator==(const string_view ifcName) const
    {
        const auto ifcNameSize = ifcName.size();
        if (std::memcmp(this->name_, ifcName.data(), ifcNameSize) == 0)
        {
            const auto lastChar = this->name_[ifcName.size()];
            if (lastChar == '\0') // partially comared
                return equal::FULL;
            if (is_digit(lastChar)) // partial name must be looks like IfcName001
                return equal::PARTIAL;
        }
        return equal::ERROR;
    }

    auto operator()() const
    {
        return invoke(createFn_);
    }

    auto name_size(const string_view knownPart = {}) const
    {
#ifdef _DEBUG
        if (!knownPart.empty() && std::memcmp(this->name_, knownPart.data(), knownPart.size()) != 0)
            FD_ASSERT("Incorrect known part");
#endif

        const auto idxStart = this->name_ + knownPart.size();
        const auto idxSize = str_len(idxStart);
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
    void* ifcAddr = nullptr;

    for (auto& reg : interface_reg::range(createInterfaceFn))
    {
        const auto result = reg == name;
        if (!result)
            continue;

#ifndef _DEBUG
        if (notify)
#endif
            if (result == interface_reg::equal::PARTIAL)
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

        ifcAddr = invoke(reg);
        break;
    }

    if (notify)
        _log_found_object(this->get(), L"csgo interface", logName.empty() ? name : logName, ifcAddr);

    return ifcAddr;
}

static DECLSPEC_NOINLINE PVOID _get_current_module_handle()
{
    if (CurrentLibraryHandle == nullptr)
    {
        MEMORY_BASIC_INFORMATION info;
        constexpr SIZE_T infoSize = sizeof(MEMORY_BASIC_INFORMATION);
        // todo: is this is dll, try to load this function from inside
        [[maybe_unused]] const auto len = VirtualQueryEx(GetCurrentProcess(), _get_current_module_handle, &info, infoSize);
        FD_ASSERT(len == infoSize, "Wrong size");
        CurrentLibraryHandle = static_cast<HINSTANCE>(info.AllocationBase);
    }
    return CurrentLibraryHandle;
}

current_library_info::current_library_info(const bool notify)
    : library_info(static_cast<IMAGE_DOS_HEADER*>(_get_current_module_handle()), notify)
{
}

//----------

namespace fd
{
    HMODULE CurrentLibraryHandle;
}