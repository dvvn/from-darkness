#pragma once

#include "functional/cast.h"
#include "library_info/core.h"
#include "string/view.h"

#include <cassert>

namespace fd
{
class library_info;

namespace detail
{
struct library_object_getter_tag
{
};

struct library_object_getter : library_object_getter_tag
{
  protected:
    library_info const* linfo_;

  public:
    library_object_getter(library_info const* linfo)
        : linfo_{linfo}
    {
    }
};

template <class Getter, size_t Index = 0>
struct library_object_getter_tuple_size : library_object_getter_tuple_size<Getter, Index + 1>
{
};

template <class Getter, size_t Index>
requires(Index != 0 && requires(Getter g) { g.template get<Index>(); } == false)
struct library_object_getter_tuple_size<Getter, Index> : integral_constant<size_t, Index>
{
};

template <class Lib = library_info>
struct library_section_getter;
template <class Lib>
library_section_getter(Lib const*) -> library_section_getter<Lib>;

template <class Lib = library_info>
struct library_pattern_getter;
template <class Lib>
library_pattern_getter(Lib const*) -> library_pattern_getter<Lib>;

template <class Lib = library_info>
struct library_function_getter;
template <class Lib>
    using basic_object_getter_tag = detail::library_object_getter_tag;
    using basic_object_getter     = detail::library_object_getter;

    struct basic_function_getter;
    struct basic_section_getter;
    struct basic_pattern_getter;

library_function_getter(Lib const*) -> library_function_getter<Lib>;

template <class Lib = native_library_info>
struct library_interface_getter;
template <class Lib>
library_interface_getter(Lib const*) -> library_interface_getter<Lib>;

template <>
struct library_section_getter<library_info> : library_object_getter
{
    using pointer = IMAGE_SECTION_HEADER const*;

    pointer find(string_view name) const;

    pointer rdata() const
    {
        return find(".rdata");
    }

    pointer text() const
    {
        return find(".text");
    }
};

template <>
struct library_pattern_getter<library_info> : library_section_getter<>
{
    template <class... Segment>
    auto find(pattern<Segment...> const& pat) const -> void*;
};

template <>
struct library_function_getter<library_info> : library_object_getter
{
    void* find(string_view name) const;
};

template <>
struct library_function_getter<native_library_info> : library_function_getter<>
{
    void* create_interface() const
    {
        return find("CreateInterface");
    }
};

template <>
struct library_interface_getter<native_library_info> : library_function_getter<native_library_info>
{
    safe_cast_lazy<void*> get(string_view name /*= native::interface_name<T>::value*/) const;

  private:
    native::interface_register* root_interface() const;
    using library_function_getter::find;
};

template <class Lib = library_info>
struct library_section_getter;
template <class Lib>
library_section_getter(Lib const*) -> library_section_getter<Lib>;

template <class Lib = library_info>
struct library_pattern_getter;
template <class Lib>
library_pattern_getter(Lib const*) -> library_pattern_getter<Lib>;

template <class Lib = library_info>
struct library_function_getter;
template <class Lib>
library_function_getter(Lib const*) -> library_function_getter<Lib>;

template <class Lib = native_library_info>
struct library_interface_getter;
template <class Lib>
library_interface_getter(Lib const*) -> library_interface_getter<Lib>;

template <>
struct library_section_getter<library_info> : library_object_getter
{
    using pointer = IMAGE_SECTION_HEADER const*;

    pointer find(string_view name) const;

    pointer rdata() const
    {
        return find(".rdata");
    }

    pointer text() const
    {
        return find(".text");
    }
};

template <>
struct library_pattern_getter<library_info> : library_section_getter<>
{
    template <class... Segment>
    auto find(pattern<Segment...> const& pat) const -> void*;
};

template <>
struct library_function_getter<library_info> : library_object_getter
{
    void* find(string_view name) const;
};

template <>
struct library_function_getter<native_library_info> : library_function_getter<>
{
    void* create_interface() const
    {
        return find("CreateInterface");
    }
};

template <>
struct library_interface_getter<native_library_info> : library_function_getter<native_library_info>
{
    safe_cast_lazy<void*> get(string_view name /*= native::interface_name<T>::value*/) const;

  private:
    native::interface_register* root_interface() const;
    using library_function_getter::find;
};
} // namespace detail

class library_info
{
    union
    {
        LDR_DATA_TABLE_ENTRY_FULL* entry_full_;
        LDR_DATA_TABLE_ENTRY* entry_;
    };

    static LIST_ENTRY* module_list();
    static LDR_DATA_TABLE_ENTRY_FULL* ldr_table(LIST_ENTRY* entry);

  protected:
    template <class... T>
    class packed_objects
    {
        void* buff_[sizeof...(T)];

      public:
        packed_objects(T*... obj)
            : buff_{obj...}
        {
        }

        template <class Out>
        operator Out() const
#ifdef _DEBUG
            requires(std::constructible_from<Out, T*...>)
#endif
        {
            return [&]<size_t... I>(std::index_sequence<I...>) -> Out {
                return {static_cast<T*>(buff_[I])...};
            }(std::make_index_sequence<sizeof...(T)>{});
        }
    };

  public:
  protected:
    struct basic_function_getter;
    class basic_interface_getter;

  public:
    library_info(wstring_view name);
    library_info(wstring_view name, wstring_view ext);

    template <std::derived_from<library_info> Other>
    library_info(Other other);

    explicit operator bool() const
    {
        return entry_ != nullptr;
    }

    IMAGE_DOS_HEADER* dos_header() const
    {
        auto const dos = entry_full_->DosHeader;
        assert(dos->e_magic == IMAGE_DOS_SIGNATURE);
        return dos;
    }

    IMAGE_NT_HEADERS* nt_header() const
    {
        auto const dos = dos_header();
        auto const nt  = unsafe_cast<IMAGE_NT_HEADERS*>(entry_full_->DllBaseAddress + dos->e_lfanew);
        assert(nt->Signature == IMAGE_NT_SIGNATURE);
        return nt;
    }

    void* image_base() const
    {
        auto const nt = nt_header();
        return unsafe_cast_from(nt->OptionalHeader.ImageBase);
    }

    /*uint8_t* begin() const
    {
        return safe_cast_from(entry_full_->DllBase);
    }

    uint8_t* end() const
    {
        return unsafe_cast_from(entry_full_->DllBaseAddress + entry_full_->SizeOfImage);
    }*/

    uint8_t* data() const
    {
        return safe_cast_from(entry_full_->DllBase);
    }

    ULONG size() const
    {
        return entry_full_->SizeOfImage;
    }

    wstring_view name() const
    {
        using std::data;
        using std::size;
        auto const& buff = entry_full_->BaseDllName;
        return {data(buff), size(buff)};
    }

    wstring_view path() const
    {
        using std::data;
        using std::size;
        auto const& buff = entry_full_->FullDllName;
        return {data(buff), size(buff)};
    }

    // void* vtable(string_view name) const;
};

#if 0
inline void* library_info::vtable(string_view name) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void* rtti_descriptor;
        char const* rtti_descriptor_view;
    };

    // rtti_descriptor = find_rtti_descriptor(name, image_base, nt_header);
    if (auto const space = name.find(' '); space == name.npos)
    {
        rtti_descriptor = find(make_pattern(".?A", 1, name, 0, "@@"));
    }
    else
    {
        auto const object_name = name.substr(0, space);
        char object_tag;

        if (object_name == "struct")
            object_tag = 'U';
        else if (object_name == "class")
            object_tag = 'V';
        else
            unreachable();

        auto const class_name = name.substr(space + 1);

        rtti_descriptor = find(make_pattern(".?A", 0, object_tag, 0, class_name, 0, "@@"));
    }

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor{rtti_descriptor_address - sizeof(uintptr_t) * 2};

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto const img_base = safe_cast<uint8_t>(image_base());

    auto const rdata       = find(section::rdata);
    auto const rdata_first = img_base + rdata->VirtualAddress;
    auto const rdata_last  = rdata_first + rdata->SizeOfRawData;

    auto const addr1 = std::find_if(rdata_first, rdata_last, [&type_descriptor](uint8_t const& found) -> bool {
        return std::equal(type_descriptor.begin(), type_descriptor.end(), &found) && *unsafe_cast<uint32_t*>(&found - 0x8) == 0;
    });
    if (addr1 == rdata_last)
        return nullptr;
    xref const unnamed_xref2{unsafe_cast<uintptr_t>(addr1) - 0xC};
    auto const addr2 = std::search(rdata_first, rdata_last, unnamed_xref2.begin(), unnamed_xref2.end());
    if (addr2 == rdata_last)
        return nullptr;

    auto const text       = find(section::text);
    auto const text_first = img_base + text->VirtualAddress;
    auto const text_last  = text_first + text->SizeOfRawData;
    xref const unnamed_xref3{unsafe_cast<uintptr_t>(addr2) + 4};
    auto const addr3 = std::search(text_first, text_last, unnamed_xref3.begin(), unnamed_xref3.end());
    if (addr3 == text_last)
        return nullptr;

    return (addr3);
}
#endif

class native_library_info : public library_info
{
    using library_info::library_info;
};
} // namespace fd

template <size_t I, std::derived_from<fd::detail::library_object_getter_tag> Getter>
struct std::tuple_element<I, Getter> : type_identity<decltype(declval<Getter>().template get<I>())>
{
};

template <std::derived_from<fd::detail::library_object_getter_tag> Getter>
struct std::tuple_size<Getter> : fd::detail::library_object_getter_tuple_size<Getter>
{
};
