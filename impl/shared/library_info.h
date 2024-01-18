#pragma once

#include "functional/cast.h"

#include <windows.h>
#include <winternl.h>

#include <string_view>

// ReSharper disable CppInconsistentNaming

// see https://www.vergiliusproject.com/kernels/x64/Windows%2011/22H2%20(2022%20Update)/_LDR_DATA_TABLE_ENTRY
typedef struct _LDR_DATA_TABLE_ENTRY_FULL
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;

    union
    {
        PVOID DllBase;
        PIMAGE_DOS_HEADER DosHeader;
        ULONG_PTR DllBaseAddress;
    };

    PVOID EntryPoint;

    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_FULL, *PLDR_DATA_TABLE_ENTRY_FULL;

// ReSharper restore CppInconsistentNaming

namespace fd
{
namespace detail
{
template <class L>
class library_object_getter;

template <size_t I, class L>
void get(library_object_getter<L> const&);

template <class ObjGetter, size_t I, typename GetResult = decltype(get<I>(std::declval<ObjGetter>()))>
struct library_object_getter_tuple_element : std::type_identity<GetResult>
{
};

template <class ObjGetter, size_t I>
struct library_object_getter_tuple_element<ObjGetter, I, void>;

template <class ObjGetter, size_t I = 0, typename GetResult = decltype(get<I>(std::declval<ObjGetter>()))>
struct library_object_getter_tuple_size;

template <class ObjGetter>
struct library_object_getter_tuple_size<ObjGetter, 0, void>;

template <class ObjGetter, size_t I>
struct library_object_getter_tuple_size<ObjGetter, I, void> : std::integral_constant<size_t, I>
{
};

template <class ObjGetter, size_t I, typename GetResult>
struct library_object_getter_tuple_size<ObjGetter, I, GetResult*> : library_object_getter_tuple_size<ObjGetter, I + 1>
{
};

template <typename CharT, size_t Length, class Config>
class basic_static_string_full;
} // namespace detail

class library_info final
{
    union
    {
        PLDR_DATA_TABLE_ENTRY_FULL entry_full_;
        PLDR_DATA_TABLE_ENTRY entry_;
    };

#if 0
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
#endif

  public:
    library_info(PLDR_DATA_TABLE_ENTRY_FULL entry);
    library_info(PLDR_DATA_TABLE_ENTRY entry);

    [[deprecated]]
    library_info(wchar_t const* name, size_t length);

    library_info(std::wstring_view name);
    library_info(std::wstring_view name, std::wstring_view extension);

    explicit operator bool() const;

    IMAGE_DOS_HEADER* dos_header() const;
    IMAGE_NT_HEADERS* nt_header() const;
    void* image_base() const;

    /*uint8_t* begin() const
    {
        return safe_cast_from(entry_full_->DllBase);
    }

    uint8_t* end() const
    {
        return unsafe_cast_from(entry_full_->DllBaseAddress + entry_full_->SizeOfImage);
    }*/

    uint8_t* data() const;
    ULONG size() const;

    std::wstring_view name() const;
    std::wstring_view path() const;

    // void* vtable(std::string_view name) const;
};

inline namespace literals
{
// #ifdef _DEBUG
// inline library_info operator"" _dll(wchar_t const* name, size_t const length)
//{
//     return {
//         std::wstring_view{name, length},
//         L".dll"
//     };
// }
// #else
// template <constant_wstring Name>
// library_info operator"" _dll()
//{
//     return {Name + L".dll"};
// }
// #endif

library_info operator"" _dll(wchar_t const* name, size_t length);
} // namespace literals

} // namespace fd

template <size_t I, class Info>
struct std::tuple_element<I, fd::detail::library_object_getter<Info>> :
    fd::detail::library_object_getter_tuple_element<fd::detail::library_object_getter<Info>, I>
{
};

template <class Info>
struct std::tuple_size<fd::detail::library_object_getter<Info>> : fd::detail::library_object_getter_tuple_size<fd::detail::library_object_getter<Info>>
{
};
