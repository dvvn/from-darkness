module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <typeinfo>

module fd.rt_modules:find_vtable;
import :find_section;
import :library_info;
import fd.logger;
import fd.signature;
import fd.functional;
import fd.chars_cache;

using namespace fd;

template <typename Fn>
static auto _Get_cross_references(signature_finder finder, const uintptr_t addr, Fn callback)
{
    bool stop        = false;
    auto& [from, to] = finder;
    do
    {
        const auto xref = finder((uint8_t*)&addr, sizeof(uintptr_t));

        // it means that there either were no xrefs, or there are no remaining xrefs
        if (!xref)
            break;

        callback(reinterpret_cast<uintptr_t>(xref), stop);
        if (stop)
            break;

        from += sizeof(uintptr_t);
    }
    while (from <= to);
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

static void* _Find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const string_view name, const obj_type type, const bool notify)
{
    if (!ldr_entry)
        return nullptr;

    const dos_nt dnt(ldr_entry);
    const auto memory_span = dnt.read();
    const signature_finder whole_module_finder(memory_span.data(), memory_span.size());

    constexpr chars_cache raw_prefix  = ".?A";
    constexpr chars_cache raw_postfix = "@@";

    void* rtti_class_name;

    if (type == TYPE_UNKNOWN)
    {
        constexpr auto bytes_prefix  = _Bytes_to_sig<raw_prefix>();
        const auto bytes_name        = _Bytes_to_sig(name.data(), name.size());
        constexpr auto bytes_postfix = _Bytes_to_sig<raw_postfix>();

        const auto real_name_unk = make_string(bytes_prefix, " ? ", bytes_name, ' ', bytes_postfix);
        rtti_class_name          = whole_module_finder(real_name_unk);
    }
    else if (type == TYPE_NATIVE)
    {
        const auto ptr = (const uint8_t*)name.data();
        FD_ASSERT(ptr >= whole_module_finder.from && ptr <= whole_module_finder.to - name.size(), "Selected wrong module!");
        rtti_class_name = (void*)ptr;
    }
    else
    {
        char str_prefix;
        if (type == TYPE_CLASS)
            str_prefix = 'V';
        else if (type == TYPE_STRUCT)
            str_prefix = 'U';
        else
            FD_ASSERT_UNREACHABLE("Unknown type");

        const auto real_name = make_string(raw_prefix, str_prefix, name, raw_postfix);
        rtti_class_name      = whole_module_finder(real_name, true);
    }

    FD_ASSERT(rtti_class_name != nullptr);

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_class_name);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
    type_descriptor -= sizeof(uintptr_t) * 2;

    const auto dot_rdata = find_section(ldr_entry, ".rdata");
    const auto dot_text  = find_section(ldr_entry, ".text");

    const signature_finder dot_rdata_finder(dnt.map(dot_rdata->VirtualAddress), dot_rdata->SizeOfRawData);
    const signature_finder dot_text_finder(dnt.map(dot_text->VirtualAddress), dot_text->SizeOfRawData);

    void* vtable_ptr = nullptr;
    _Get_cross_references(dot_rdata_finder, type_descriptor, [&](const uintptr_t xref, bool& stop) {
        // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
        const auto vtable_offset = *reinterpret_cast<uint32_t*>(xref - 0x8);
        if (vtable_offset != 0)
            return;

        const auto object_locator = xref - 0xC;
        const auto vtable_address = (uintptr_t)dot_rdata_finder((uint8_t*)&object_locator, sizeof(uintptr_t)) + 0x4;

        // check is valid offset
        if (vtable_address > sizeof(uintptr_t))
            vtable_ptr = dot_text_finder((uint8_t*)&vtable_address, sizeof(uintptr_t));

        stop = true;
    });
    if (notify)
        library_info(ldr_entry).log("vtable", name, vtable_ptr);
    else
        FD_ASSERT(vtable_ptr != nullptr);
    return vtable_ptr;
}

void* find_vtable_class(LDR_DATA_TABLE_ENTRY* const ldr_entry, const string_view name, const bool notify)
{
    return _Find_vtable(ldr_entry, name, TYPE_CLASS, notify);
}

void* find_vtable_struct(LDR_DATA_TABLE_ENTRY* const ldr_entry, const string_view name, const bool notify)
{
    return _Find_vtable(ldr_entry, name, TYPE_STRUCT, notify);
}

void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const string_view name, const bool notify)
{
    const auto do_find = bind_front(_Find_vtable, ldr_entry);

    constexpr string_view class_prefix = "class ";
    if (name.starts_with(class_prefix))
        return do_find(name.substr(class_prefix.size()), TYPE_CLASS, notify);

    constexpr string_view struct_prefix = "struct ";
    if (name.starts_with(struct_prefix))
        return do_find(name.substr(struct_prefix.size()), TYPE_STRUCT, notify);

    return do_find(name, TYPE_UNKNOWN, notify);
}

void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::type_info& info, const bool notify)
{
    const auto raw_name = info.raw_name();
    string_view name;
    if (notify)
        name = raw_name;
    else
        name = { raw_name, 0 };
    return _Find_vtable(ldr_entry, name, TYPE_NATIVE, notify);
}
