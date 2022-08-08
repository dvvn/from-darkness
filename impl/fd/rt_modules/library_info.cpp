module;

#include <fd/assert.h>

#include <fd/rt_modules/winapi.h>

#include <memory>
#include <span>
#include <typeinfo>

module fd.rt_modules:library_info;
import fd.logger;
import fd.path;
import fd.mem_scanner;
import fd.chars_cache;
import fd.ctype;
import fd.address; //todo : remove

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

template <typename Fn>
static LDR_DATA_TABLE_ENTRY* _Find_library(Fn comparer)
{
    const auto mem =
#if defined(_M_X64) || defined(__x86_64__)
        NtCurrentTeb();
    FD_ASSERT(mem != nullptr, "Teb not found");
    const auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
        reinterpret_cast<PEB*>(__readfsdword(0x30));
    FD_ASSERT(mem != nullptr, "Peb not found");
    const auto ldr        = mem->Ldr;
#endif
    // get module linked list.
    const auto list = &ldr->InMemoryOrderModuleList;
    // iterate linked list.
    for (auto it = list->Flink; it != list; it = it->Flink)
    {
        // get current entry.
        const auto ldr_entry = CONTAINING_RECORD(it, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (!ldr_entry)
            continue;

        const auto [dos, nt] = dos_nt(ldr_entry);
        if (!comparer(ldr_entry, dos, nt))
            continue;

        return ldr_entry;
    }

    return nullptr;
}

library_info::library_info(pointer const entry)
    : entry_(entry)
{
    FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(const wstring_view name, const bool notify)
    : entry_(_Find_library([=](const library_info info, void*, void*) {
        return info.name() == name;
    }))
{
    if (notify)
        invoke(logger, L"{} -> {}! ({:#X})", name, entry_ ? L"found" : L"not found", reinterpret_cast<uintptr_t>(entry_));
    else
        FD_ASSERT(entry_ != nullptr);
}

library_info::library_info(IMAGE_DOS_HEADER* const base_address, const bool notify)
    : entry_(_Find_library([=](void*, IMAGE_DOS_HEADER* const dos, void*) {
        return base_address == dos;
    }))
{
    if (notify)
    {
        const auto get_name = [=] {
            return this->entry_ ? this->name() : L"unknown name";
        };
        invoke(logger,
               L"{:#X} ({}) -> {}! ({:#X})",
               reinterpret_cast<uintptr_t>(base_address),
               get_name,
               entry_ ? L"found" : L"not found",
               reinterpret_cast<uintptr_t>(entry_) /*    */);
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

wstring_view library_info::name() const
{
    const auto full_path = this->path();
#if 1
    return fd::path<wchar_t>(full_path).filename();
#else
    const auto name_start = full_path.rfind('\\');
    FD_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
#endif
}

constexpr auto _Object_found = [](const library_info* info, const auto object_type, const auto object, const void* addr) {
    invoke(logger,
           L"{} -> {} '{}' {}! ({:#X})",
           bind_front(&library_info::name, info),
           object_type,
           object,
           addr ? L"found" : L"not found",
           reinterpret_cast<uintptr_t>(addr) /*            */);
};

void library_info::log_class_info(const fd::string_view raw_name, const void* addr) const
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
        const auto object      = raw_name.substr(name_begin + 1);

        invoke(notification, object_type, object);
    }
}

void* library_info::find_export(const string_view name, const bool notify) const
{
    FD_ASSERT_UNREACHABLE("Not implemented");
#if 0
    if (!ldr_entry)
        return nullptr;
    // base address
    const dos_nt dnt(ldr_entry);

    // get export data directory.
    const auto& data_dir = dnt.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    FD_ASSERT(data_dir.VirtualAddress, "Current module doesn't have the virtual address!");

    // get export export_dir.
    const basic_address<IMAGE_EXPORT_DIRECTORY> export_dir = dos + data_dir.VirtualAddress;

    // names / funcs / ordinals ( all of these are RVAs ).
    uint32_t* const names = dos + export_dir->AddressOfNames;
    uint32_t* const funcs = dos + export_dir->AddressOfFunctions;
    uint16_t* const ords  = dos + export_dir->AddressOfNameOrdinals;

    void* export_ptr = nullptr;

    // iterate names array.
    for (size_t i = 0; i < export_dir->NumberOfNames; ++i)
    {
        const char* export_name = dos + names[i];
        if (!export_name)
            continue;
        if (export_name != name)
            continue;
        /*
   if (export_addr.cast<uintptr_t>() >= reinterpret_cast<uintptr_t>(export_directory)
      && export_addr.cast<uintptr_t>() < reinterpret_cast<uintptr_t>(export_directory) + data_directory->Size)
   */

        // if (export_ptr < export_dir || export_ptr >= memory_block(export_dir, data_dir.Size).addr( ))
        const auto temp_export_ptr = dos + funcs[ords[i]];
        if (temp_export_ptr < export_dir || temp_export_ptr >= export_dir + data_dir.Size)
        {
            export_ptr = temp_export_ptr;
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
        library_info(ldr_entry).log("export", name, export_ptr);

    return export_ptr;
#endif
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

enum obj_type : uint8_t
{
    TYPE_UNKNOWN,
    TYPE_CLASS,
    TYPE_STRUCT,
    TYPE_NATIVE
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
extern "C" char* __cdecl __unDName(char* outputString,
                                   const char* name,
                                   int maxStringLength, // Note, COMMA is leading following optional arguments
                                   allocation_function pAlloc,
                                   free_function pFree,
                                   uint16_t disableFlags);

static string _Demangle_symbol(const char* mangled_name)
{
    constexpr allocation_function alloc = [](size_t size) {
        return static_cast<void*>(new char[size]);
    };
    constexpr free_function free_f = [](void* p) {
        auto chr = static_cast<char*>(p);
        delete[] chr;
    };

    const std::unique_ptr<char> name(__unDName(nullptr, mangled_name + 1, 0, alloc, free_f, 0x2800 /*UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY*/));

    return name.get();
}

static void* _Find_vtable(const library_info info, const string_view name, const obj_type type, const bool notify)
{
    const dos_nt dnt(info);
    const auto memory_span = dnt.read();
    const pattern_scanner whole_module_finder(memory_span.data(), memory_span.size());

    constexpr chars_cache raw_prefix  = ".?A";
    constexpr chars_cache raw_postfix = "@@";

    constexpr auto class_prefix  = 'V';
    constexpr auto struct_prefix = 'U';

    void* rtti_class_name;

    if (type == TYPE_UNKNOWN)
    {
        constexpr auto bytes_prefix  = _Bytes_to_sig<raw_prefix>();
        const auto bytes_name        = _Bytes_to_sig(name.data(), name.size());
        constexpr auto bytes_postfix = _Bytes_to_sig<raw_postfix>();

        const auto real_name_unk = make_string(bytes_prefix, " ? ", bytes_name, ' ', bytes_postfix);
        rtti_class_name          = whole_module_finder(real_name_unk).front();
    }
    else if (type == TYPE_NATIVE)
    {
        const auto ptr  = (const uint8_t*)name.data();
        // FD_ASSERT(ptr >= whole_module_finder.from && ptr <= whole_module_finder.to - name.size(), "Selected wrong module!");
        rtti_class_name = (void*)ptr;
    }
    else
    {
        char str_prefix;
        if (type == TYPE_CLASS)
            str_prefix = class_prefix;
        else if (type == TYPE_STRUCT)
            str_prefix = struct_prefix;
        else
            FD_ASSERT_UNREACHABLE("Unknown type");

        const auto real_name = make_string(raw_prefix, str_prefix, name, raw_postfix);
        rtti_class_name      = whole_module_finder(real_name, raw_pattern).front();
    }

    FD_ASSERT(rtti_class_name != nullptr); //------------------------------

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_class_name);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
    type_descriptor -= sizeof(uintptr_t) * 2;

    const auto dot_rdata = info.find_section(".rdata");
    const auto dot_text  = info.find_section(".text");

    const xrefs_finder dot_rdata_finder(dnt.map(dot_rdata->VirtualAddress), dot_rdata->SizeOfRawData);
    const xrefs_finder dot_text_finder(dnt.map(dot_text->VirtualAddress), dot_text->SizeOfRawData);

    void* vtable_ptr = nullptr;

    for (const auto xref : dot_rdata_finder(type_descriptor))
    {
        // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
        const auto vtable_offset = *reinterpret_cast<uint32_t*>(xref - 0x8);
        if (vtable_offset != 0)
            continue;

        const auto object_locator = xref - 0xC;
        const auto vtable_address = dot_rdata_finder(object_locator).front() + 0x4;

        // check is valid offset
        FD_ASSERT(vtable_address > sizeof(uintptr_t));
        vtable_ptr = (void*)dot_text_finder(vtable_address).front();
        break;
    }

    if (notify)
    {
#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

        const auto demagle_type = [=]() -> string_view {
            const auto prefix = reinterpret_cast<const char*>(rtti_class_name)[raw_prefix.size()];
            if (prefix == class_prefix)
                return "class";
            else if (prefix == struct_prefix)
                return "struct";
            else
                FD_ASSERT_UNREACHABLE("Unknown prefix!");
        };

        const auto demagle_name = [=]() -> string {
            string buff;
            if (name.contains('@'))
                buff = _Demangle_symbol(reinterpret_cast<const char*>(rtti_class_name));
            else if (name.contains(' '))
                buff = name;
            else
                return { name.data(), name.size() };

            return buff.substr(buff.find(' ') + 1);
        };

        invoke(logger,
               L"{} -> {} {} '{}' {}! ({:#X})",
               bind_front(&library_info::name, &info),
               L"vtable for",
               demagle_type,
               demagle_name,
               vtable_ptr ? L"found" : L"not found",
               reinterpret_cast<uintptr_t>(vtable_ptr));
    }

    return vtable_ptr;
}

void* library_info::find_vtable_class(const string_view name, const bool notify) const
{
    return _Find_vtable(entry_, name, TYPE_CLASS, notify);
}

void* library_info::find_vtable_struct(const string_view name, const bool notify) const
{
    return _Find_vtable(entry_, name, TYPE_STRUCT, notify);
}

void* library_info::find_vtable_unknown(const string_view name, const bool notify) const
{
    return _Find_vtable(entry_, name, TYPE_UNKNOWN, notify);
}

void* library_info::find_vtable(const string_view name, const bool notify) const
{
    const auto do_find = bind_back(bind_front(_Find_vtable, entry_), notify);

    constexpr string_view class_prefix = "class ";
    if (name.starts_with(class_prefix))
        return do_find(name.substr(class_prefix.size()), TYPE_CLASS);

    constexpr string_view struct_prefix = "struct ";
    if (name.starts_with(struct_prefix))
        return do_find(name.substr(struct_prefix.size()), TYPE_STRUCT);

    return do_find(name, TYPE_UNKNOWN);
}

void* library_info::find_vtable(const std::type_info& info, const bool notify) const
{
    const auto raw_name = info.raw_name();
    string_view name;
    if (notify)
        name = raw_name;
    else
        name = { raw_name, 0 };
    return _Find_vtable(entry_, name, TYPE_NATIVE, notify);
}

struct interface_reg
{
    void* (*create_fn)();
    const char* name;
    const interface_reg* next;

    //--------------

    const interface_reg* find(const string_view interface_name) const
    {
        for (auto reg = this; reg != nullptr; reg = reg->next)
        {
            const auto name_size = interface_name.size();
            if (std::memcmp(interface_name.data(), reg->name, name_size) != 0)
                continue;
            const auto last_char = reg->name[interface_name.size()];
            if (last_char != '\0') // partially comared
            {
                if (!is_digit(last_char)) // must be looks like IfcName001
                    continue;
#ifdef _DEBUG
                const auto idx_start = reg->name + name_size;
                const auto idx_size  = str_len(idx_start);
                if (idx_size > 1)
                    FD_ASSERT(is_digit(idx_start + 1, idx_start + idx_size));
                if (reg->next)
                    FD_ASSERT(!reg->next->find(interface_name));
#endif
            }
            return reg;
        }

        return nullptr;
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

    const interface_reg* root_reg = basic_address(create_interface_fn)./*rel32*/ jmp(0x5).plus(0x6).deref<2>();
    const auto target_reg         = root_reg->find(name);
    FD_ASSERT(target_reg != nullptr);
    const auto ifc_addr = invoke(target_reg->create_fn);

    if (notify)
        _Object_found(this, L"csgo interface", target_reg->name, ifc_addr);

    return ifc_addr;
}
