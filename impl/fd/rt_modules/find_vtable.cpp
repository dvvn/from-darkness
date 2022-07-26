module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <typeinfo>

module fd.rt_modules:find_vtable;
import :find_section;
import :helpers;
import :library_info;
import fd.address;
import fd.logger;
import fd.signature;
import fd.functional;
import fd.chars_cache;

using namespace fd;

template <typename Fn>
static auto _Get_cross_references(signature_finder finder, const uintptr_t addr, Fn callback)
{
    auto& [from, to] = finder;
    do
    {
        const auto xref = finder((uint8_t*)&addr, sizeof(uintptr_t));

        // it means that there either were no xrefs, or there are no remaining xrefs
        if (!xref)
            break;

        bool stop = false;
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
        pattern[i]      = hex_digits[((curr_byte & 0xF0) >> 4)];
        pattern[i + 1U] = hex_digits[(curr_byte & 0x0F)];
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

    const auto [dos, nt] = dos_nt(ldr_entry);

    void* rtti_class_name = nullptr;
    const signature_finder whole_module_finder(dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage);

    constexpr auto class_prefix  = 'V';
    constexpr auto struct_prefix = 'U';

    const auto do_find = [&](const auto str_prefix) {
        constexpr chars_cache raw_prefix  = ".?A";
        constexpr chars_cache raw_postfix = "@@";

        constexpr auto bytes_prefix  = _Bytes_to_sig<raw_prefix>();
        constexpr auto bytes_postfix = _Bytes_to_sig<raw_postfix>();

        if (str_prefix == '?')
        {
            const auto real_name_unk = make_string(bytes_prefix, " ? ", _Bytes_to_sig(name.data(), name.size()), ' ', bytes_postfix);
            rtti_class_name          = whole_module_finder(real_name_unk);
        }
        else
        {
            const auto real_name = make_string(raw_prefix, str_prefix, name, raw_postfix);
            rtti_class_name      = whole_module_finder(real_name, true);
        }
        return rtti_class_name != nullptr;
    };

    switch (type)
    {
    case TYPE_UNKNOWN: {
#if 1
        // still not perfect but better than double scan
        if (do_find('?'))
            goto _FOUND;
#else
        if (do_find(class_prefix))
            goto _FOUND;
        if (do_find(struct_prefix))
            goto _FOUND;
#endif
        break;
    }
    case TYPE_CLASS: {
        if (do_find(class_prefix))
            goto _FOUND;
        break;
    }
    case TYPE_STRUCT: {
        if (do_find(struct_prefix))
            goto _FOUND;
        break;
    }
    case TYPE_NATIVE: {
        const auto ptr = (const uint8_t*)name.data();
        FD_ASSERT(ptr >= whole_module_finder.from && ptr <= whole_module_finder.to - name.size(), "Selected wrong module!");
        rtti_class_name = (void*)ptr;
        goto _FOUND;
    }
    };

    FD_ASSERT(rtti_class_name != nullptr);

_FOUND:

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_class_name);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
    type_descriptor -= sizeof(uintptr_t) * 2;

    const auto dot_rdata = find_section(ldr_entry, ".rdata");
    const auto dot_text  = find_section(ldr_entry, ".text");

    const signature_finder dot_rdata_finder(dos + dot_rdata->VirtualAddress, dot_rdata->SizeOfRawData);
    const signature_finder dot_text_finder(dos + dot_text->VirtualAddress, dot_text->SizeOfRawData);

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
