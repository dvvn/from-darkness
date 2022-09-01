module;

#include <fd/utility.h>

#include <memory>
#include <utility>

export module fd.rt_modules;
export import :library_info;
import fd.chars_cache;
import fd.ctype;

using fd::chars_cache;
using fd::library_info;
using fd::wstring_view;

bool _Wait_for_library(const wstring_view name);

struct any_module_base
{
    virtual ~any_module_base() = default;

    virtual const library_info& data() const = 0;
    virtual bool wait() const                = 0;

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
    bool wait() const override;
};

class magic_cast
{
    union
    {
        void* ptr_;
        uintptr_t addr_;
    };

  public:
    magic_cast(void* ptr)
        : ptr_(ptr)
    {
    }

    magic_cast(const uintptr_t addr)
        : addr_(addr)
    {
    }

    template <typename Q>
    operator Q*() const // requires(std::is_pointer_v<Q> || std::is_member_function_pointer_v<Q> || std::is_function_v<Q>)
    {
        return reinterpret_cast<Q*>(addr_);
    }
};

auto _Correct_signature(void* root_addr, const size_t add, size_t deref)
{
    auto addr = reinterpret_cast<uintptr_t>(root_addr) + add;
    do
    {
        const auto new_addr = *reinterpret_cast<void**>(addr);
        addr                = reinterpret_cast<uintptr_t>(new_addr);
    }
    while (deref-- > 0);
    return reinterpret_cast<void*>(addr);
}

template <chars_cache Name, size_t Idx>
struct rt_module final : any_module<Idx /* fd::_Hash_bytes(Name.data(), Name.size()) */>
{
    const library_info& data() const override
    {
        static library_info info = [this] {
#ifdef _DEBUG
            if (!_Wait_for_library(Name))
            {
                // handle error and unload!
            }
#endif
            return Name.view();
        }();

        return info;
    }

    bool wait() const override
    {
        return _Wait_for_library(Name);
    }

    template <chars_cache Interface>
    magic_cast find_interface() const
    {
        static const auto found = this->data().find_csgo_interface(this->find_export<"CreateInterface">(), Interface);
        return found;
    }

  private:
    template <chars_cache Sig, size_t Add, size_t Deref>
    auto _Find_interface_sig() const
    {
        static const auto found = _Correct_signature(this->find_signature<Sig>(), Add, Deref);
        return found;
    }

  public:
    template <chars_cache Sig, size_t Add, size_t Deref, class T>
    auto find_interface_sig() const
    {
        static const auto found = [this] {
            const magic_cast addr =
#ifdef _DEBUG
                _Find_interface_sig<Sig, Add, Deref>()
#else
                _Correct_signature(this->find_signature<Sig>(), Add, Deref)
#endif
                ;
            T* result = addr;
            this->data().log_class_info(result);
            return result;
        }();
        return found;
    }

    template <chars_cache Sig, size_t Add, size_t Deref, chars_cache DebugName>
    magic_cast find_interface_sig() const
    {
        static const auto found = [this] {
            const auto addr =
#ifdef _DEBUG
                _Find_interface_sig<Sig, Add, Deref>()
#else
                _Correct_signature(this->find_signature<Sig>(), Add, Deref)
#endif
                ;
            this->data().log_class_info(DebugName, addr);
            return addr;
        }();
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
