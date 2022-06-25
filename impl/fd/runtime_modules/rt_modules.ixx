module;

#include <windows.h>
#include <winternl.h>

#include <memory>
#include <string>

export module fd.rt_modules;
import :find_library;
import :find_export;
import :find_section;
import :find_vtable;
import :find_signature;
import :find_csgo_interface;
export import fd.address;
export import fd.type_name;
import fd.chars_cache;

using fd::basic_address;

/* struct logs_writer
{
    void operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::wstring_view module_name) const;

    template <typename FnT>
    void operator()(const found_export<FnT> ex, const std::wstring_view module_name, const std::string_view export_name) const
    {
        console_log(module_name, "export", export_name, ex.pointer);
    }

    void operator()(IMAGE_SECTION_HEADER* const sec, const std::wstring_view module_name, const std::string_view section_name) const;

    template <typename T>
    void operator()(const found_vtable<T> vt, const std::wstring_view module_name, const std::string_view vtable_name) const
    {
        console_log(module_name, "vtable", vtable_name, vt.ptr);
    }
}; */

template <class Gm>
class interface_finder
{
    [[no_unique_address]] Gm rt_module_;
    basic_address<void>      addr_;

  public:
    interface_finder(const basic_address<void> addr, Gm&& gm = {})
        : addr_(addr)
        , rt_module_(std::move(gm))
    {
    }

    template <class T>
    operator T*() const
    {
        T* const ptr = addr_;
        std::invoke(fd::on_csgo_interface_found, rt_module_.data(), fd::type_name<T>(), ptr);
        return ptr;
    }

    template <size_t Idx>
    interface_finder& deref()
    {
        addr_ = addr_.deref<Idx>();
        return *this;
    }

    interface_finder& plus(ptrdiff_t offset)
    {
        addr_ = addr_.plus(offset);
        return *this;
    }

    interface_finder& minus(ptrdiff_t offset)
    {
        addr_ = addr_.minus(offset);
        return *this;
    }

    interface_finder& multiply(ptrdiff_t value)
    {
        addr_ = addr_.multiply(value);
        return *this;
    }

    interface_finder& divide(ptrdiff_t value)
    {
        addr_ = addr_.divide(value);
        return *this;
    }

    interface_finder& operator[](ptrdiff_t value)
    {
        addr_ = addr_[value];
        return *this;
    }
};

template <class Gm>
interface_finder(Gm) -> interface_finder<Gm>;

// todo: store chars_cache in type_name
template <class T>
constexpr auto cache_type_name = [] {
    constexpr auto name = fd::type_name<T>();
    return fd::chars_cache<char, name.size() + 1>(name.data(), name.size());
}();

namespace fd
{
    template <chars_cache Name>
    struct rt_module
    {
        interface_finder<rt_module> _Ifc_finder(const basic_address<void> addr) const
        {
            return addr;
        }

        //---

        auto data() const
        {
            static const auto found = fd::find_library(Name);
            return found;
        }

        template <chars_cache ExpName>
        basic_address<void> find_export() const
        {
            static const auto found = fd::find_export(this->data(), ExpName);
            return found;
        }

        template <chars_cache Interface>
        basic_address<void> find_interface() const
        {
            static const auto found = fd::find_csgo_interface(this->find_export<"CreateInterface">(), Interface, this->data());
            return found;
        }

        template <chars_cache Sig>
        basic_address<void> find_signature() const
        {
            static const auto found = fd::find_signature(this->data(), Sig);
            return found;
        }

        template <chars_cache Sig>
        auto find_interface_sig() const
        {
            return _Ifc_finder(this->find_signature<Sig>());
        }

        template <chars_cache Name>
        void* find_vtable() const
        {
            static const auto found = fd::find_vtable(this->data(), Name);
            return found;
        }

        template <typename T>
        T* find_vtable() const
        {
            static_assert(std::is_class_v<T> /* is_abstract_v */);
            return static_cast<T*>(find_vtable<cache_type_name<T>>());
        }
    };
} // namespace fd

struct current_module
{
    std::wstring_view                _Name() const;
    interface_finder<current_module> _Ifc_finder(const basic_address<void> addr) const;
};

// clang-format off
#define DLL_NAME(_NAME_)    L""#_NAME_".dll"
#define GAME_MODULE(_NAME_) constexpr rt_module<DLL_NAME(_NAME_)> _NAME_;
// clang-format on

#define EXPORT_PROXY(_NAME_) using fd::on_##_NAME_##_found

namespace export_proxy
{
    EXPORT_PROXY(csgo_interface);
    EXPORT_PROXY(export);
    EXPORT_PROXY(library);
    EXPORT_PROXY(section);
    EXPORT_PROXY(signature);
    EXPORT_PROXY(vtable);
} // namespace export_proxy

export namespace fd
{
    using namespace export_proxy;

    namespace runtime_modules
    {
        constexpr current_module current;

        GAME_MODULE(server);
        GAME_MODULE(client);
        GAME_MODULE(engine);
        GAME_MODULE(datacache);
        GAME_MODULE(materialsystem);
        GAME_MODULE(vstdlib);
        GAME_MODULE(vgui2);
        GAME_MODULE(vguimatsurface);
        GAME_MODULE(vphysics);
        GAME_MODULE(inputsystem);
        GAME_MODULE(studiorender);
        GAME_MODULE(shaderapidx9);
    } // namespace runtime_modules
} // namespace fd