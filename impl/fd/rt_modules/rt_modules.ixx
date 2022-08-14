module;

#include <fd/utility.h>

#include <memory>
#include <utility>

export module fd.rt_modules;
export import fd.type_name;
import :library_info;
import fd.chars_cache;
import fd.ctype;

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
    auto find_export() const
    {
        static const auto found = this->data().find_export(ExpName);
        return found;
    }

    template <chars_cache Sig>
    auto find_signature() const
    {
        static const auto found = this->data().find_signature(Sig);
        return found;
    }

    template <chars_cache Name>
    auto find_vtable() const
    {
        static const auto found = this->data().find_vtable(Name);
        return found;
    }

    template <class T>
    auto find_vtable() const
    {
        static const auto found = this->data().find_vtable<T>();
        return found;
    }
};

struct current_module final : any_module<0>
{
    const library_info& data() const override;
    bool loaded() const override;
};

union library_info_lazy
{
    void* dummy = nullptr;
    library_info info;
};

class magic_cast
{
    void* ptr_;

  public:
    magic_cast(void* ptr)
        : ptr_(ptr)
    {
    }

    template <typename Q>
    operator Q*() const // requires(std::is_pointer_v<Q> || std::is_member_function_pointer_v<Q> || std::is_function_v<Q>)
    {
        return static_cast<Q*>(ptr_);
    }
};

template <chars_cache Name, size_t Idx>
class rt_module final : public any_module<Idx /* fd::_Hash_bytes(Name.data(), Name.size()) */>
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
    magic_cast find_interface() const
    {
        static const auto found = this->data().find_csgo_interface(this->find_export<"CreateInterface">(), Interface);
        return found;
    }
};

template <size_t S>
constexpr auto _To_lower_buff(const wchar_t (&name)[S])
{
    chars_cache buff = name;
    fd::to_lower(name);
    return buff;
}

#define DLL_NAME(_NAME_)    L"" #_NAME_ ".dll"
#define GAME_MODULE(_NAME_) constexpr rt_module<_To_lower_buff(DLL_NAME(_NAME_)), __COUNTER__ + 1> _NAME_;

export namespace fd::rt_modules
{
    constexpr current_module current;

    FOR_EACH(GAME_MODULE,
             server,
             client,
             engine,
             dataCache,
             materialSystem,
             vstdlib,
             vgui2,
             vguiMatSurface,
             vphysics,
             inputSystem,
             studioRender,
             shaderApiDx9,
             d3d9,
             serverBrowser /*                */
    );
} // namespace fd::rt_modules
