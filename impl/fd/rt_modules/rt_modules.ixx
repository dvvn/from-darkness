module;

#include <fd/utility.h>

#include <memory>
#include <utility>

export module fd.rt_modules;
export import :library_info;
import fd.chars_cache;
import fd.ctype;

using namespace fd;

library_info _Wait_for_library(const wstring_view name);

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

struct current_module final : any_module<-1>
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

class unknown_module : public any_module_base
{
    wstring name_;
    mutable library_info info_;

  public:
    unknown_module(const wstring_view name, const bool exact = false, const string_view extension = ".dll");

    unknown_module(const unknown_module&)            = delete;
    unknown_module& operator=(const unknown_module&) = delete;

    unknown_module(unknown_module&& other);
    unknown_module& operator=(unknown_module&& other);

    const library_info& data() const override;
    bool wait() const override;

    //---

    template <chars_cache ExpName>
    auto find_export() const
    {
        return this->data().find_export(ExpName);
    }

    template <chars_cache Sig>
    auto find_signature() const
    {
        return this->data().find_signature(Sig);
    }

    template <chars_cache Name>
    auto find_vtable() const
    {
        return this->data().find_vtable(Name);
    }

    template <class T>
    auto find_vtable() const
    {
        return this->data().find_vtable<T>();
    }
};

template <chars_cache Name, size_t Idx>
struct known_module final : any_module<Idx /* fd::_Hash_bytes(Name.data(), Name.size()) */>
{
    const library_info& data() const override
    {
        static library_info info =
#ifdef _DEBUG
            _Wait_for_library(Name)
#else
            Name.view()
#endif
            ;

        return info;
    }

    bool wait() const override
    {
        return static_cast<bool>(_Wait_for_library(Name));
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
    chars_cache<wchar_t, S + 4> buff;
    auto itr = buff.begin();
    for (auto c : name)
        *itr++ = to_lower(c);
    std::copy_n(L".dll", 4, itr);
    return buff;
}

#define DLL_NAME(_NAME_) L"" #_NAME_ /* ".dll" */
#define EXTERNAL_MODULE(_NAME_)                                                       \
    constexpr known_module<_To_lower_buff(DLL_NAME(_NAME_)), __COUNTER__ + 1> _NAME_; \
    constexpr auto& _NAME_##_fn()                                                     \
    {                                                                                 \
        return _NAME_;                                                                \
    }

export namespace fd::rt_modules
{
    constexpr current_module current;

    using get = unknown_module;

    FOR_EACH(EXTERNAL_MODULE,
             // system modules
             d3d9,
             ntDll);

    FOR_EACH(EXTERNAL_MODULE,
             // game modules
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
             serverBrowser);
} // namespace fd::rt_modules
