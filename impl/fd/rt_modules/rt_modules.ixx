module;

#include <fd/utility.h>

#include <windows.h>
#include <winternl.h>

#include <memory>

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
import fd.functional;

using fd::basic_address;

template <class Gm>
class interface_finder
{
    [[no_unique_address]] Gm rt_module_;
    basic_address<void> addr_;

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
        fd::invoke(fd::on_csgo_interface_found, rt_module_.data(), fd::type_name<T>(), ptr);
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
            static const auto found = find_library(Name);
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

        template <class T>
        T* find_vtable() const
        {
            static const auto found = fd::find_vtable<T>(this->data());
            return found;
        }
    };

    struct current_module
    {
        wstring_view _Name() const;

        LDR_DATA_TABLE_ENTRY* operator->() const;
        LDR_DATA_TABLE_ENTRY* data() const;

        template <chars_cache ExpName>
        basic_address<void> find_export() const
        {
            static const auto found = fd::find_export(this->data(), ExpName);
            return found;
        }

        template <chars_cache Sig>
        basic_address<void> find_signature() const
        {
            static const auto found = fd::find_signature(this->data(), Sig);
            return found;
        }

        template <chars_cache Name>
        void* find_vtable() const
        {
            static const auto found = fd::find_vtable(this->data(), Name);
            return found;
        }

        template <class T>
        T* find_vtable() const
        {
            static const auto found = fd::find_vtable<T>(this->data());
            return found;
        }
    };

} // namespace fd

// clang-format off
#define DLL_NAME(_NAME_)    L""#_NAME_".dll"
#define GAME_MODULE(_NAME_) constexpr rt_module<DLL_NAME(_NAME_)> _NAME_;

// clang-format on

export namespace fd
{
    namespace rt_modules
    {
        constexpr current_module current;

        FOR_EACH(GAME_MODULE, server, client, engine, datacache, materialsystem, vstdlib, vgui2, vguimatsurface, vphysics, inputsystem, studiorender, shaderapidx9, d3d9);

    } // namespace rt_modules

} // namespace fd
