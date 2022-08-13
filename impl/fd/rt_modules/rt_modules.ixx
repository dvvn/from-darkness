module;

#include <fd/utility.h>

#include <memory>
#include <utility>

export module fd.rt_modules;
export import fd.address;
export import fd.type_name;
import :library_info;
import fd.chars_cache;

using fd::address;
using fd::chars_cache;
using fd::library_info;

struct only_true
{
    only_true(const bool val);
};

struct any_module_base
{
    virtual ~any_module_base() = default;

    virtual const library_info& data() const = 0;
    virtual bool loaded() const              = 0;

    const library_info* operator->() const;
    const library_info& operator*() const;
};

template <size_t UniqueNum>
struct any_module : any_module_base
{
    template <chars_cache ExpName>
    void* find_export() const
    {
        static const auto found = this->data().find_export(ExpName);
        return found;
    }

    template <chars_cache Sig>
    address<void> find_signature() const
    {
        static const auto found = this->data().find_signature(Sig);
        return found;
    }

    template <chars_cache Name>
    void* find_vtable() const
    {
        static const auto found = this->data().find_vtable(Name);
        return found;
    }

    template <class T>
    T* find_vtable() const
    {
        static const auto found = this->data().find_vtable<T>();
        return found;
    }
};

union library_info_lazy
{
    void* dummy = nullptr;
    library_info info;
};

template <chars_cache Name>
class rt_module final : public any_module<fd::_Hash_object(Name)>
{
    static auto& _Data()
    {
        static library_info_lazy val;
        return val;
    }

    static bool _Loaded()
    {
        auto& info = _Data().info;
        if (!info.get())
            std::construct_at(&info, Name);
        return info.get();
    }

  public:
    const library_info& data() const override
    {
        static const only_true dummy = _Loaded();
        (void)dummy;
        return _Data().info;
    }

    bool loaded() const override
    {
        return _Loaded();
    }

    template <chars_cache Interface>
    address<void> find_interface() const
    {
        static const auto found = this->data().find_csgo_interface(this->find_export<"CreateInterface">(), Interface);
        return found;
    }
};

struct current_module final : any_module<0>
{
    const library_info& data() const override;
    bool loaded() const override;
};

#define DLL_NAME(_NAME_)    L"" #_NAME_ ".dll"
#define GAME_MODULE(_NAME_) constexpr rt_module<DLL_NAME(_NAME_)> _NAME_;

export namespace fd::rt_modules
{
    constexpr current_module current;

    FOR_EACH(GAME_MODULE,
             server,
             client,
             engine,
             datacache,
             materialsystem,
             vstdlib,
             vgui2,
             vguimatsurface,
             vphysics,
             inputsystem,
             studiorender,
             shaderapidx9,
             d3d9,
             serverbrowser);
} // namespace fd::rt_modules
