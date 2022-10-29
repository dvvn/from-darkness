module;

#include <fd/assert.h>

#include "dll_notification.h"

#include <windows.h>
#include <winternl.h>

#include <algorithm>
#include <array>
#include <span>
#include <typeinfo>

module fd.library_info;
import fd.logger;
import fd.filesystem.path;
import fd.mem_scanner;
import fd.string.info;
import fd.functional.bind;
import fd.string.make;
import fd.demangle_symbol;
import fd.semaphore;

using namespace fd;

void dos_nt::_Construct(const LDR_DATA_TABLE_ENTRY* ldr_entry)
{
    FD_ASSERT(ldr_entry != nullptr);
    dos = (IMAGE_DOS_HEADER*)ldr_entry->DllBase;
    // check for invalid DOS / DOS signature.
    FD_ASSERT(dos && dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    nt = map<IMAGE_NT_HEADERS>(dos->e_lfanew);
    // check for invalid NT / NT signature.
    FD_ASSERT(nt && nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
}

dos_nt::dos_nt(const LDR_DATA_TABLE_ENTRY* ldr_entry)
{
    _Construct(ldr_entry);
}

dos_nt::dos_nt(const library_info info)
{
    _Construct(info.get());
}

PVOID dos_nt::base() const
{
    return dos;
}

std::span<uint8_t> dos_nt::read() const
{
    return { (uint8_t*)base(), nt->OptionalHeader.SizeOfImage };
}

std::span<IMAGE_SECTION_HEADER> dos_nt::sections() const
{
    return { IMAGE_FIRST_SECTION(nt), nt->FileHeader.NumberOfSections };
}

//---------

class LIST_ENTRY_iterator
{
    LIST_ENTRY* current_;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = size_t;
    using value_type        = LIST_ENTRY;
    using pointer           = value_type*;
    using reference         = value_type&;

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
        auto tmp = *this;
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
        auto tmp = *this;
        operator--();
        return tmp;
    }

    LIST_ENTRY& operator*() const
    {
        return *current_;
    }

    bool operator==(const LIST_ENTRY_iterator& other) const = default;
};

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
    template <typename T>
    T* get(LIST_ENTRY& list)
    {
        return CONTAINING_RECORD(&list, T, InMemoryOrderLinks);
    }

    template <typename T>
    const T* get(const LIST_ENTRY& list)
    {
        return CONTAINING_RECORD(&list, T, InMemoryOrderLinks);
    }
} // namespace std

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

static wstring_view _To_string_view(const UNICODE_STRING& ustr)
{
    return { ustr.Buffer, ustr.Length / sizeof(WCHAR) };
}

static wstring_view _Library_info_path(const LDR_DATA_TABLE_ENTRY* entry)
{
    return _To_string_view(entry->FullDllName);
}

static wstring_view _Library_info_name(const LDR_DATA_TABLE_ENTRY* entry)
{
    const auto full_path = _Library_info_path(entry);
#if 1
    return fs::basic_path(full_path).filename();
#else
    const auto name_start = full_path.rfind('\\');
    FD_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
#endif
}

static wstring_view _Found_or_not(const void* ptr)
{
    return ptr ? L"found" : L"not found";
}

static void _Log_found_entry(const wstring_view name, const LDR_DATA_TABLE_ENTRY* entry)
{
    invoke(logger,
           //
           L"{} -> {}! ({:#X})",
           name,
           bind_front(_Found_or_not, entry),
           reinterpret_cast<uintptr_t>(entry));
}

static void _Log_found_entry(const IMAGE_DOS_HEADER* base_address, const LDR_DATA_TABLE_ENTRY* entry)
{
    const auto get_name = [=] {
        return entry ? _Library_info_name(entry) : L"unknown name";
    };
    invoke(logger,
           //
           L"{:#X} ({}) -> {}! ({:#X})",
           reinterpret_cast<uintptr_t>(base_address),
           get_name,
           bind_front(_Found_or_not, entry),
           reinterpret_cast<uintptr_t>(entry));
}

constexpr auto _Log_found_object = [](const LDR_DATA_TABLE_ENTRY* entry, const auto object_type, const auto object, const void* addr) {
    invoke(logger,
           //
           L"{} -> {} '{}' {}! ({:#X})",
           bind_front(_Library_info_name, entry),
           object_type,
           object,
           bind_front(_Found_or_not, addr),
           reinterpret_cast<uintptr_t>(addr));
};

static void _Log_address_found(const LDR_DATA_TABLE_ENTRY* entry, const string_view raw_name, const void* addr)
{
    if (!logger)
        return;

    const auto _Log_found_object_ex = [&](const auto object_type, const auto object) {
        _Log_found_object(entry, object_type, object, addr);
    };

    const auto name_begin = raw_name.find(' '); // class or struct
    if (name_begin == raw_name.npos)
    {
        _Log_found_object_ex(L"object", raw_name);
    }
    else
    {
        const auto object_type = raw_name.substr(0, name_begin);
        FD_ASSERT(!object_type.empty(), "Wrong object type");
        const auto object = raw_name.substr(name_begin + 1);
        FD_ASSERT(!object.empty(), "Wrong object name");
        _Log_found_object_ex(object_type, object);
    }
}

template <typename T>
constexpr auto _Bytes_to_sig(const T* bytes, const size_t size)
{
    constexpr auto hex_digits = "0123456789ABCDEF";
    const auto hex_length     = (size << 1) + size;

    // construct pre-reserved string filled with spaces
    string pattern(hex_length - 1, ' ');

    for (size_t i = 0, n = 0; i < hex_length; ++n, i += 3)
    {
        const uint8_t curr_byte = bytes[n];

        // manually convert byte to chars
        pattern[i]     = hex_digits[((curr_byte & 0xF0) >> 4)];
        pattern[i + 1] = hex_digits[(curr_byte & 0x0F)];
    }

    return pattern;
}

template <size_t S>
constexpr auto _Bytes_to_sig(const std::array<char, S>& str)
{
    constexpr auto str_size = S - 1;
    constexpr auto size     = (str_size << 1) + str_size;
    const auto tmp          = _Bytes_to_sig(str.data(), str_size);
    std::array<char, size> buff{};
    std::copy(tmp.begin(), tmp.end(), buff.begin());
    return buff;
}

namespace std
{
    template <size_t S>
    array(const char (&)[S]) -> array<char, S>;
}

struct rtti_info
{
    static constexpr std::array raw_prefix  = { ".?A" };
    static constexpr std::array raw_postfix = { "@@" };

    static constexpr auto raw_prefix_bytes  = _Bytes_to_sig(raw_prefix);
    static constexpr auto raw_postfix_bytes = _Bytes_to_sig(raw_prefix);

    static constexpr auto class_prefix  = 'V';
    static constexpr auto struct_prefix = 'U';
};

static void _Log_found_vtable(const LDR_DATA_TABLE_ENTRY* entry, const string_view name, const char* type_descriptor, const void* vtable_ptr)
{
#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

    const auto demagle_type = [=]() -> string_view {
        const auto prefix = type_descriptor[rtti_info::raw_prefix.size()];
        if (prefix == rtti_info::class_prefix)
            return "class";
        else if (prefix == rtti_info::struct_prefix)
            return "struct";
        else
            FD_ASSERT_UNREACHABLE("Unknown prefix!");
    };

    const auto demagle_name = [=]() -> string {
        string buff;
        if (name.contains('@'))
            buff = demangle_symbol(type_descriptor);
        else if (name.contains(' '))
            buff = name;
        else
            return { name.data(), name.size() };

        return buff.substr(buff.find(' ') + 1);
    };

    invoke(logger,
           L"{} -> {} {} '{}' {}! ({:#X})",
           bind_front(_Library_info_name, entry),
           L"vtable for",
           demagle_type,
           demagle_name,
           bind_front(_Found_or_not, vtable_ptr),
           reinterpret_cast<uintptr_t>(vtable_ptr));
}

library_info library_info::_Find(const wstring_view name, const bool notify)
{
    const auto entry = LDR_ENTRY_finder([=](const library_info info) {
        return info.name() == name;
    });
#ifdef _DEBUG
    if (!entry)
        return {}; // prevent assert
    else
#endif
        if (notify && entry)
        _Log_found_entry(name, entry);
    return entry;
}

struct callback_data_t
{
    wstring_view name;
    semaphore sem = { 0, 1 };
    PVOID found;
};

static void CALLBACK _On_new_library(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context)
{
    const auto data = static_cast<callback_data_t*>(Context);

    if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_UNLOADED)
    {
        if (NotificationData->Unloaded.DllBase != current_library_info(false)->DllBase)
            return;
    }
    else
    {
#if 0
        const auto target_name = _To_string_view(*NotificationData->Loaded.FullDllName);
        if (!target_name.ends_with(data->name))
            return;
#else
        if (_To_string_view(*NotificationData->Loaded.BaseDllName) != data->name)
            return;
#endif
        data->found = NotificationData->Loaded.DllBase;
    }

    data->sem.release();
}

static auto _Wait_prepare(const bool notify)
{
    const library_info ntdll = { L"ntdll.dll", false, notify };

    const auto reg   = static_cast<LdrRegisterDllNotification>(ntdll.find_export("LdrRegisterDllNotification"));
    const auto unreg = static_cast<LdrUnregisterDllNotification>(ntdll.find_export("LdrUnregisterDllNotification"));

    return std::pair(reg, unreg);
}

PVOID library_info::_Wait(const wstring_view name, const bool notify)
{
    static const auto [reg_fn, unreg_fn] = _Wait_prepare(notify);

    callback_data_t cb_data = { name };
    void* cookie;
    if (reg_fn(0, _On_new_library, &cb_data, &cookie) != STATUS_SUCCESS)
        return nullptr;
    if (!cb_data.sem.acquire())
        return nullptr;
    if (unreg_fn(cookie) != STATUS_SUCCESS)
        return nullptr;
    return cb_data.found;
}

library_info::library_info()
    : entry_(nullptr)
{
}

library_info::library_info(pointer entry)
    : entry_(entry)
{
    FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(const wstring_view name, const bool wait, const bool notify)
{
    entry_ = LDR_ENTRY_finder([=](const library_info info) {
        return info.name() == name;
    });

    if (!entry_ && wait)
    {
        const auto base_address = _Wait(name, notify);
        if (base_address)
        {
            entry_ = LDR_ENTRY_finder([=](const dos_nt dnt) {
                return base_address == dnt.base();
            });
        }
    }

    if (notify)
        _Log_found_entry(name, entry_);
    else
        FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(const IMAGE_DOS_HEADER* base_address, const bool notify)
    : entry_(LDR_ENTRY_finder([=](const dos_nt dnt) {
        return base_address == dnt.base();
    }))
{
    if (notify)
        _Log_found_entry(base_address, entry_);
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
    for (const auto& list : LIST_ENTRY_range())
        return entry_ == std::get<LDR_DATA_TABLE_ENTRY>(list);

    return false;
}

bool library_info::unload() const
{
    return FreeLibrary(reinterpret_cast<HMODULE>(entry_->DllBase));
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
    return _Library_info_path(entry_);
}

wstring_view library_info::name() const
{
    return _Library_info_name(entry_);
}

void library_info::log_class_info(const string_view raw_name, const void* addr) const
{
    _Log_address_found(entry_, raw_name, addr);
}

void* library_info::find_export(const string_view name, const bool notify) const
{
    if (!entry_)
        return nullptr;
    // base address
    const dos_nt dnt(entry_);

    // get export data directory.
    const auto& data_dir = dnt.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    FD_ASSERT(data_dir.VirtualAddress, "Current module doesn't have the virtual address!");

    // get export export_dir.
    const auto export_dir = dnt.map<const IMAGE_EXPORT_DIRECTORY>(data_dir.VirtualAddress);

    const auto export_dir_min = reinterpret_cast<const uint8_t*>(export_dir);
    const auto export_dir_max = export_dir_min + data_dir.Size;

    // names / funcs / ordinals ( all of these are RVAs ).
    const auto names = dnt.map<const uint32_t>(export_dir->AddressOfNames);
    const auto funcs = dnt.map<const uint32_t>(export_dir->AddressOfFunctions);
    const auto ords  = dnt.map<const uint16_t>(export_dir->AddressOfNameOrdinals);

    void* export_ptr = nullptr;

    // iterate names array.
    for (size_t i = 0; i < export_dir->NumberOfNames; ++i)
    {
        const auto export_name = dnt.map<const char>(names[i]);
        if (!export_name)
            continue;
        if (export_name != name)
            continue;

        const auto temp_export_ptr = dnt.map<uint8_t>(funcs[ords[i]]);
        if (temp_export_ptr < export_dir_min || temp_export_ptr >= export_dir_max)
        {
            export_ptr = reinterpret_cast<void*>(temp_export_ptr);
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
        _Log_found_object(entry_, L"export", name, export_ptr);

    return export_ptr;
}

IMAGE_SECTION_HEADER* library_info::find_section(const string_view name, const bool notify) const
{
    IMAGE_SECTION_HEADER* found_header = nullptr;

    for (auto& header : dos_nt(entry_).sections())
    {
        if (reinterpret_cast<const char*>(header.Name) == name)
        {
            found_header = &header;
            break;
        }
    }
    if (notify)
        _Log_found_object(entry_, L"section", name, found_header);
    return found_header;
}

void* library_info::find_signature(const string_view sig, const bool notify) const
{
    const auto memory_span = dos_nt(entry_).read();
    const pattern_scanner finder(memory_span.data(), memory_span.size());
    const auto result = finder(sig).front();
    if (notify)
        _Log_found_object(entry_, L"signature", sig, result);
    return result;
}

enum class obj_type : uint8_t
{
    UNKNOWN,
    CLASS,
    STRUCT,
    NATIVE
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
        const auto memory_span = dnt_.read();
        const pattern_scanner whole_module_finder(memory_span.data(), memory_span.size());

        const void* rtti_class_name;

        if (type == obj_type::UNKNOWN)
        {
            const auto bytes_name = _Bytes_to_sig(name.data(), name.size());

            const auto real_name_unk = make_string(rtti_info::raw_prefix_bytes, " ? ", bytes_name, ' ', rtti_info::raw_postfix_bytes);
            rtti_class_name          = whole_module_finder(real_name_unk).front();
        }
        else if (type == obj_type::NATIVE)
        {
            // const auto ptr  = (const uint8_t*)name.data();
            // FD_ASSERT(ptr >= whole_module_finder.from && ptr <= whole_module_finder.to - name.size(), "Selected wrong module!");
            rtti_class_name = name.data();
        }
        else
        {
            char str_prefix;
            if (type == obj_type::CLASS)
                str_prefix = rtti_info::class_prefix;
            else if (type == obj_type::STRUCT)
                str_prefix = rtti_info::struct_prefix;
            else
                FD_ASSERT_UNREACHABLE("Unknown type");

            const auto real_name = make_string(rtti_info::raw_prefix, str_prefix, name, rtti_info::raw_postfix);
            rtti_class_name      = whole_module_finder(real_name, raw_pattern).front();
        }

        return static_cast<const char*>(rtti_class_name);
    }

    void* operator()(const void* rtti_class_name) const
    {
        // get rtti type descriptor
        auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_class_name);
        // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
        type_descriptor -= sizeof(uintptr_t) * 2;

        const auto dot_rdata = info_.find_section(".rdata");
        const auto dot_text  = info_.find_section(".text");

        const xrefs_scanner dot_rdata_finder(dnt_.map(dot_rdata->VirtualAddress), dot_rdata->SizeOfRawData);
        const xrefs_scanner dot_text_finder(dnt_.map(dot_text->VirtualAddress), dot_text->SizeOfRawData);

        for (const auto xref : dot_rdata_finder(type_descriptor))
        {
            const auto val           = reinterpret_cast<uint32_t>(xref);
            // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
            const auto vtable_offset = *reinterpret_cast<uint32_t*>(val - 0x8);
            if (vtable_offset != 0)
                continue;

            const auto object_locator = val - 0xC;
            const auto vtable_address = reinterpret_cast<uintptr_t>(dot_rdata_finder(object_locator).front()) + 0x4;

            // check is valid offset
            FD_ASSERT(vtable_address > sizeof(uintptr_t));
            return dot_text_finder(vtable_address).front();
        }
        return nullptr;
    }
};

static void* _Find_vtable(const library_info info, const string_view name, const obj_type type, const bool notify)
{
    const vtable_finder finder(info);

    const auto rtti_class_name = finder.find_type_descriptor(name, type);
    FD_ASSERT(rtti_class_name != nullptr);
    const auto vtable_ptr = finder(rtti_class_name);
    if (notify)
        _Log_found_vtable(info.get(), name, rtti_class_name, vtable_ptr);

    return vtable_ptr;
}

void* library_info::find_vtable_class(const string_view name, const bool notify) const
{
    return _Find_vtable(entry_, name, obj_type::CLASS, notify);
}

void* library_info::find_vtable_struct(const string_view name, const bool notify) const
{
    return _Find_vtable(entry_, name, obj_type::STRUCT, notify);
}

void* library_info::find_vtable_unknown(const string_view name, const bool notify) const
{
    return _Find_vtable(entry_, name, obj_type::UNKNOWN, notify);
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
    return _Find_vtable(entry_, helper.name(), helper.is_class() ? obj_type::CLASS : helper.is_struct() ? obj_type::STRUCT : obj_type::UNKNOWN, notify);
}

void* library_info::find_vtable(const std::type_info& info, const bool notify) const
{
    const auto raw_name = info.raw_name();
    return _Find_vtable(entry_, { raw_name, notify ? str_len(raw_name) : 0 }, obj_type::NATIVE, notify);
}

#undef ERROR

class interface_reg
{
    void* (*create_fn)();
    const char* name_;
    const interface_reg* next_;

  public:
    class iterator
    {
        const interface_reg* current_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = size_t;

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
            auto tmp = *this;
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
        range(const void* create_interface_fn)
        {
            const auto relative_fn  = reinterpret_cast<uintptr_t>(create_interface_fn) + 0x5;
            const auto displacement = *reinterpret_cast<int32_t*>(relative_fn);
            const auto jmp          = relative_fn + sizeof(displacement) + displacement;

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
            return nullptr;
        }
    };

    struct equal
    {
        enum value_type : uint8_t
        {
            FULL,
            PARTIAL,
            ERROR
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
        };
    };

    interface_reg()                     = delete;
    interface_reg(const interface_reg&) = delete;

    equal operator==(const string_view interface_name) const
    {
        const auto ifc_name_size = interface_name.size();
        if (std::memcmp(this->name_, interface_name.data(), ifc_name_size) == 0)
        {
            const auto last_char = this->name_[interface_name.size()];
            if (last_char == '\0') // partially comared
                return equal::FULL;
            if (is_digit(last_char)) // partial name must be looks like IfcName001
                return equal::PARTIAL;
        }
        return equal::ERROR;
    }

    auto operator()() const
    {
        return invoke(create_fn);
    }

    auto name_size(const string_view known_part = {}) const
    {
#ifdef _DEBUG
        if (!known_part.empty() && std::memcmp(this->name_, known_part.data(), known_part.size()) != 0)
            FD_ASSERT("Incorrect known part");
#endif

        const auto idx_start = this->name_ + known_part.size();
        const auto idx_size  = str_len(idx_start);
        return known_part.size() + idx_size;
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
        while (--offset);

        return src;
    }
};

void* csgo_library_info::find_interface(const string_view name, const bool notify) const
{
    return find_interface(find_export("CreateInterface"), name, notify);
}

void* csgo_library_info::find_interface(const void* create_interface_fn, const string_view name, const bool notify) const
{
    if (!create_interface_fn)
        return nullptr;

    string_view log_name;
    void* ifc_addr = nullptr;

    for (auto& reg : interface_reg::range(create_interface_fn))
    {
        const auto result = reg == name;
        if (!result)
            continue;

#ifndef _DEBUG
        if (notify)
#endif
            if (result == interface_reg::equal::PARTIAL)
            {
                const auto whole_name_size = reg.name_size(name);
#ifdef _DEBUG
                if (!std::all_of(reg.name() + name.size(), reg.name() + whole_name_size, is_digit))
                    FD_ASSERT("Incorrect given interface name");
                const auto next_reg = reg + 1;
                if (next_reg)
                {
                    for (auto& reg1 : interface_reg::range(next_reg))
                    {
                        if (reg1 == name)
                            FD_ASSERT("Duplicate interface name detected");
                    }
                }
                if (notify)
#endif
                    log_name = { reg.name(), whole_name_size };
            }

        ifc_addr = invoke(reg);
        break;
    }

    if (notify)
        _Log_found_object(this->get(), L"csgo interface", log_name.empty() ? name : log_name, ifc_addr);

    return ifc_addr;
}

static DECLSPEC_NOINLINE PVOID _Get_current_module_handle()
{
    if (!current_library_handle)
    {
        MEMORY_BASIC_INFORMATION info;
        constexpr SIZE_T info_size      = sizeof(MEMORY_BASIC_INFORMATION);
        // todo: is this is dll, try to load this function from inside
        [[maybe_unused]] const auto len = VirtualQueryEx(GetCurrentProcess(), _Get_current_module_handle, &info, info_size);
        FD_ASSERT(len == info_size, "Wrong size");
        current_library_handle = static_cast<HINSTANCE>(info.AllocationBase);
    }
    return current_library_handle;
}

current_library_info::current_library_info(const bool notify)
    : library_info(static_cast<IMAGE_DOS_HEADER*>(_Get_current_module_handle()), notify)
{
}