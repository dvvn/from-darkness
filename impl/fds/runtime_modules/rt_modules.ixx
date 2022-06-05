module;

#include <windows.h>
#include <winternl.h>

#include <memory>
#include <string>

export module fds.rt_modules;
// import fds.chars_cache; //already imported, compiler bug
import :find_csgo_interface;
import :find_section;
import :find_vtable;
import :find_signature;
export import fds.address;
import fds.type_name;

using fds::basic_address;

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
        std::invoke(fds::on_csgo_interface_found, rt_module_._Name(), fds::type_name<T>(), ptr);
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

namespace fds
{
    template <chars_cache Name>
    struct rt_module
    {
        constexpr std::wstring_view _Name() const
        {
            return Name;
        }

        interface_finder<rt_module> _Ifc_finder(const basic_address<void> addr) const
        {
            return addr;
        }

        //---

        template <chars_cache Interface>
        basic_address<void> find_interface() const
        {
            return fds::find_csgo_interface<Name, Interface>();
        }

        template <chars_cache Sig>
        basic_address<void> find_signature() const
        {
            return fds::find_signature<Name, Sig>();
        }

        template <chars_cache Sig>
        auto find_interface_sig() const
        {
            return _Ifc_finder(this->find_signature<Sig>());
        }

        template <typename T>
        T* find_vtable() const
        {
            return fds::find_vtable<Name, T>();
        }
    };
} // namespace fds

struct current_module
{
    std::wstring_view                _Name() const;
    interface_finder<current_module> _Ifc_finder(const basic_address<void> addr) const;
};

#define DLL_NAME(_NAME_)    L#_NAME_ ".dll"
#define GAME_MODULE(_NAME_) constexpr rt_module<DLL_NAME(_NAME_)> _NAME_;

#define EXPORT_PROXY(_NAME_) using fds::on_##_NAME_##_found

namespace export_proxy
{
    EXPORT_PROXY(csgo_interface);
    EXPORT_PROXY(export);
    EXPORT_PROXY(library);
    EXPORT_PROXY(section);
    EXPORT_PROXY(signature);
    EXPORT_PROXY(vtable);
} // namespace export_proxy

export namespace fds
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
} // namespace fds
