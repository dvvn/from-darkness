module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <span>
#include <typeinfo>

module fd.rt_modules:library_info;
import fd.logger;
import fd.path;
import fd.mem_scanner;
import fd.chars_cache;
import fd.ctype;
import fd.memory;
import fd.functional.bind;

using namespace fd;

void dos_nt::_Construct(const LDR_DATA_TABLE_ENTRY* const ldr_entry)
{
    FD_ASSERT(ldr_entry != nullptr);
    dos = (IMAGE_DOS_HEADER*)ldr_entry->DllBase;
    // check for invalid DOS / DOS signature.
    FD_ASSERT(dos && dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    nt = map<IMAGE_NT_HEADERS>(dos->e_lfanew);
    // check for invalid NT / NT signature.
    FD_ASSERT(nt && nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
}

dos_nt::dos_nt(const LDR_DATA_TABLE_ENTRY* const ldr_entry)
{
    _Construct(ldr_entry);
}

dos_nt::dos_nt(const library_info info)
{
    _Construct(info.get());
}

std::span<uint8_t> dos_nt::read() const
{
    return { (uint8_t*)dos, nt->OptionalHeader.SizeOfImage };
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

template <typename T, typename Fn>
T* LIST_ENTRY_finder(Fn fn)
{
    for (auto& list : LIST_ENTRY_range())
    {
        auto item = CONTAINING_RECORD(&list, T, InMemoryOrderLinks);
        if (invoke(fn, item))
            return item;
    }
    return nullptr;
}

template <typename Fn>
auto LDR_ENTRY_finder(Fn fn)
{
    return LIST_ENTRY_finder<LDR_DATA_TABLE_ENTRY>(fn);
}

library_info::library_info(pointer const entry)
    : entry_(entry)
{
    FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(const wstring_view name, const bool notify)
    : entry_(LDR_ENTRY_finder([=](const library_info info) {
        return info.name() == name;
    }))
{
    if (notify)
        invoke(logger, L"{} -> {}! ({:#X})", name, entry_ ? L"found" : L"not found", reinterpret_cast<uintptr_t>(entry_));
    else
        FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(IMAGE_DOS_HEADER* const base_address, const bool notify)
    : entry_(LDR_ENTRY_finder([=](const dos_nt dnt) {
        return base_address == dnt.dos;
    }))
{
    if (notify)
    {
        const auto get_name = [=] {
            return this->entry_ ? this->name() : L"unknown name";
        };
        invoke(logger, L"{:#X} ({}) -> {}! ({:#X})", reinterpret_cast<uintptr_t>(base_address), get_name, entry_ ? L"found" : L"not found", reinterpret_cast<uintptr_t>(entry_));
    }
    else
        FD_ASSERT(entry_ != nullptr);
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

wstring_view library_info::path() const
{
    return { entry_->FullDllName.Buffer, entry_->FullDllName.Length / sizeof(WCHAR) };
}

// to resilve 'path' word conflicts
static auto _Path_filename(const wstring_view full_path)
{
#if 1
    return path<wchar_t>(full_path).filename();
#else
    const auto name_start = full_path.rfind('\\');
    FD_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
#endif
}

wstring_view library_info::name() const
{
    return _Path_filename(this->path());
}

constexpr auto _Object_found = [](const library_info* info, const auto object_type, const auto object, const void* addr) {
    invoke(logger,
           L"{} -> {} '{}' {}! ({:#X})",
           bind_front(&library_info::name, info),
           object_type,
           object,
           addr ? L"found" : L"not found",
           reinterpret_cast<uintptr_t>(addr) /*----*/);
};

void library_info::log_class_info(const string_view raw_name, const void* addr) const
{
    const auto name_begin   = raw_name.find(' '); // class or struct
    const auto notification = bind_back(bind_front(_Object_found, this), addr);
    if (name_begin == raw_name.npos)
    {
        invoke(notification, L"object", raw_name);
    }
    else
    {
        const auto object_type = raw_name.substr(0, name_begin);
        FD_ASSERT(!object_type.empty(), "Wrong object type");
        const auto object = raw_name.substr(name_begin + 1);
        FD_ASSERT(!object.empty(), "Wrong object name");
        invoke(notification, object_type, object);
    }
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
        _Object_found(this, L"export", name, export_ptr);

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
        _Object_found(this, L"section", name, found_header);
    return found_header;
}

void* library_info::find_signature(const string_view sig, const bool notify) const
{
    const auto memory_span = dos_nt(entry_).read();
    const pattern_scanner finder(memory_span.data(), memory_span.size());
    const auto result = finder(sig).front();
    if (notify)
        _Object_found(this, L"signature", sig, result);
    return result;
}

enum class obj_type : uint8_t
{
    UNKNOWN,
    CLASS,
    STRUCT,
    NATIVE
};

template <typename T>
static constexpr auto _Bytes_to_sig(const T* bytes, const size_t size)
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

template <chars_cache Str>
static consteval auto _Bytes_to_sig()
{
    constexpr auto size = (Str.size() << 1) + Str.size(); //_Bytes_to_sig(Str.data(), Str.size()).size();
    const auto tmp      = _Bytes_to_sig(Str.data(), Str.size());
    chars_cache<char, size /* + 1 */> buff(tmp.data());
    return buff;
}

typedef void*(__cdecl* allocation_function)(size_t);
typedef void(__cdecl* free_function)(void*);
extern "C" char* __cdecl __unDName(char* outputString, const char* name, int maxStringLength, allocation_function pAlloc, free_function pFree, uint16_t disableFlags);

static string _Demangle_symbol(const char* mangled_name)
{
    constexpr allocation_function alloc = [](size_t size) {
        return static_cast<void*>(new char[size]);
    };
    constexpr free_function free_f = [](void* p) {
        auto chr = static_cast<char*>(p);
        delete[] chr;
    };

    const unique_ptr name = __unDName(nullptr, mangled_name + 1, 0, alloc, free_f, 0x2800 /*UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY*/);
    return name.get();
}

class vtable_finder
{
    static constexpr chars_cache raw_prefix_  = ".?A";
    static constexpr chars_cache raw_postfix_ = "@@";

    static constexpr auto class_prefix_  = 'V';
    static constexpr auto struct_prefix_ = 'U';

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
            constexpr auto bytes_prefix  = _Bytes_to_sig<raw_prefix_>();
            const auto bytes_name        = _Bytes_to_sig(name.data(), name.size());
            constexpr auto bytes_postfix = _Bytes_to_sig<raw_postfix_>();

            const auto real_name_unk = make_string(bytes_prefix, " ? ", bytes_name, ' ', bytes_postfix);
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
                str_prefix = class_prefix_;
            else if (type == obj_type::STRUCT)
                str_prefix = struct_prefix_;
            else
                FD_ASSERT_UNREACHABLE("Unknown type");

            const auto real_name = make_string(raw_prefix_, str_prefix, name, raw_postfix_);
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

    void notify(const string_view name, const char* type_descriptor, const void* vtable_ptr) const
    {
#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

        const auto demagle_type = [=]() -> string_view {
            const auto prefix = type_descriptor[raw_prefix_.size()];
            if (prefix == class_prefix_)
                return "class";
            else if (prefix == struct_prefix_)
                return "struct";
            else
                FD_ASSERT_UNREACHABLE("Unknown prefix!");
        };

        const auto demagle_name = [=]() -> string {
            string buff;
            if (name.contains('@'))
                buff = _Demangle_symbol(type_descriptor);
            else if (name.contains(' '))
                buff = name;
            else
                return { name.data(), name.size() };

            return buff.substr(buff.find(' ') + 1);
        };

        invoke(logger,
               L"{} -> {} {} '{}' {}! ({:#X})",
               bind_front(&library_info::name, &info_),
               L"vtable for",
               demagle_type,
               demagle_name,
               vtable_ptr ? L"found" : L"not found",
               reinterpret_cast<uintptr_t>(vtable_ptr));
    }
};

static void* _Find_vtable(const library_info info, const string_view name, const obj_type type, const bool notify)
{
    const vtable_finder finder(info);

    const auto rtti_class_name = finder.find_type_descriptor(name, type);
    FD_ASSERT(rtti_class_name != nullptr);
    const auto vtable_ptr = finder(rtti_class_name);
    if (notify)
        finder.notify(name, rtti_class_name, vtable_ptr);
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

/* void* library_info::find_vtable(const string_view name, const bool notify) const
{
    const auto do_find = bind_back(bind_front(_Find_vtable, entry_), notify);

    constexpr string_view class_prefix_ = "class ";
    if (name.starts_with(class_prefix_))
        return do_find(name.substr(class_prefix_.size()), obj_type::CLASS);

    constexpr string_view struct_prefix_ = "struct ";
    if (name.starts_with(struct_prefix_))
        return do_find(name.substr(struct_prefix_.size()), obj_type::STRUCT);

    return do_find(name, obj_type::UNKNOWN);
} */

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
        const auto name_size = interface_name.size();
        if (std::memcmp(this->name_, interface_name.data(), name_size) == 0)
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

void* library_info::find_csgo_interface(const string_view name, const bool notify) const
{
    return find_csgo_interface(find_export("CreateInterface"), name, notify);
}

void* library_info::find_csgo_interface(const void* create_interface_fn, const string_view name, const bool notify) const
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
                if (!is_digit(reg.name() + name.size(), reg.name() + whole_name_size))
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
        _Object_found(this, L"csgo interface", log_name.empty() ? name : log_name, ifc_addr);

    return ifc_addr;
}
