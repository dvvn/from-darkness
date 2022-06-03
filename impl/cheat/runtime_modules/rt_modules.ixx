module;

#include <windows.h>
#include <winternl.h>

#include <memory>
#include <string>

export module cheat.rt_modules;
import cheat.tools.object_name;
import nstd.text.chars_cache; //already imported, compiler bug
import nstd.winapi.exports;
import nstd.winapi.sections;
import nstd.winapi.vtables;
export import nstd.mem.address;

using nstd::mem::basic_address;
namespace wp = nstd::winapi;

void console_log(const std::wstring_view module_name, const std::string_view object_type, const std::string_view object_name, const basic_address<void> object_ptr);

struct logs_writer
{
    void operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::wstring_view module_name) const;

    template <typename FnT>
    void operator()(const wp::found_export<FnT> ex, const std::wstring_view module_name, const std::string_view export_name) const
    {
        console_log(module_name, "export", export_name, ex.pointer);
    }

    void operator()(IMAGE_SECTION_HEADER* const sec, const std::wstring_view module_name, const std::string_view section_name) const;

    template <typename T>
    void operator()(const wp::found_vtable<T> vt, const std::wstring_view module_name, const std::string_view vtable_name) const
    {
        console_log(module_name, "vtable", vtable_name, vt.ptr);
    }
};

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig);
void* find_interface_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const basic_address<void> create_interface_fn, const std::string_view name);

using nstd::text::chars_cache;

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
        console_log(rt_module_._Name(), "interface", cheat::object_name<T>, ptr);
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

    template <typename FnT, chars_cache Export>
    FnT find_export() const
    {
        return wp::find_export<FnT, Name, Export, logs_writer>();
    }

    template <chars_cache Section>
    auto find_section() const
    {
        return wp::find_section<Name, Section, logs_writer>();
    }

    template <typename T>
    T* find_vtable() const
    {
        static const auto found = wp::find_vtable<logs_writer>(wp::find_module<Name, logs_writer>(), this->_Name(), cheat::object_name<T>);
        return static_cast<T*>(found);
    }

    template <chars_cache Sig>
    basic_address<void> find_signature() const
    {
        static const auto found = find_signature_impl(wp::find_module<Name, logs_writer>(), Sig);
        return found;
    }

    template <chars_cache IfcName>
    basic_address<void> find_interface() const
    {
        static const auto found = find_interface_impl(wp::find_module<Name, logs_writer>(), this->find_export<void*, "CreateInterface">(), IfcName);
        return found;
    }

    template <chars_cache Sig>
    auto find_interface_sig() const
    {
        return _Ifc_finder(this->find_signature<Sig>());
    }
};

struct current_module
{
    std::wstring_view _Name() const;
    interface_finder<current_module> _Ifc_finder(const basic_address<void> addr) const;
};

#define DLL_NAME(_NAME_)          L#_NAME_ ".dll"
#define CHEAT_GAME_MODULE(_NAME_) constexpr rt_module<DLL_NAME(_NAME_)> _NAME_;

export namespace cheat::runtime_modules
{
    constexpr current_module current;

    CHEAT_GAME_MODULE(server);
    CHEAT_GAME_MODULE(client);
    CHEAT_GAME_MODULE(engine);
    CHEAT_GAME_MODULE(datacache);
    CHEAT_GAME_MODULE(materialsystem);
    CHEAT_GAME_MODULE(vstdlib);
    CHEAT_GAME_MODULE(vgui2);
    CHEAT_GAME_MODULE(vguimatsurface);
    CHEAT_GAME_MODULE(vphysics);
    CHEAT_GAME_MODULE(inputsystem);
    CHEAT_GAME_MODULE(studiorender);
    CHEAT_GAME_MODULE(shaderapidx9);
} // namespace cheat::runtime_modules
