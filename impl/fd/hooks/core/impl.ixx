module;

#include <fd/core/assert.h>
#include <fd/core/object.h>

#include <concepts>
#include <functional>
#include <string>
#include <vector>

export module fd.hook;
export import fd.hook_base;
import fd.mem_protect;

enum class call_cvs : uint8_t
{
    thiscall__,
    cdecl__,
    fastcall__,
    stdcall__,
    vectorcall__
};

template <call_cvs, typename Ret, typename... Args>
struct tiny_helper;

#define TINY_HELPER(_C_)                                \
    template <typename Ret, typename... Args>           \
    struct tiny_helper<call_cvs::_C_##__, Ret, Args...> \
    {                                                   \
        Ret __##_C_ callback(Args... args) const        \
        {                                               \
            if constexpr (!std::is_void_v<Ret>)         \
                return *(Ret*)nullptr;                  \
        }                                               \
    };

#define TINY_SELECTOR(_C_)                                                                                     \
    template <typename Ret, class T, typename... Args>                                                         \
    constexpr tiny_helper<call_cvs::_C_##__, Ret, Args...> _Tiny_selector(Ret (__##_C_ T::*fn)(Args...))       \
    {                                                                                                          \
        return {};                                                                                             \
    }                                                                                                          \
    template <typename Ret, class T, typename... Args>                                                         \
    constexpr tiny_helper<call_cvs::_C_##__, Ret, Args...> _Tiny_selector(Ret (__##_C_ T::*fn)(Args...) const) \
    {                                                                                                          \
        return {};                                                                                             \
    }

#define TINY_IMPL(_C_) TINY_HELPER(_C_) TINY_SELECTOR(_C_)

TINY_IMPL(thiscall);
TINY_IMPL(cdecl);
TINY_IMPL(fastcall);
TINY_IMPL(stdcall);
TINY_IMPL(vectorcall);

//#define FUNCTION_GETTER_HAVE_PTR_SIZE

class function_getter
{
    void* fn_ptr_     = nullptr;
#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
    uint8_t ptr_size_ = 0;
#endif

  public:
    operator void*() const
    {
        return fn_ptr_;
    }

#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE

    uint8_t size() const
    {
        return ptr_size_;
    }
#endif

    void* get() const
    {
        return fn_ptr_;
    }

    function_getter() = default;

    template <typename Fn>
    function_getter(Fn fn)
    {
        fn_ptr_   = reinterpret_cast<void*&>(fn);
#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
        ptr_size_ = sizeof(fn);
#endif
    }

    template <class C, class Fn = void*>
    function_getter(C* instance, const size_t index, Fn = {})
    {
        if (!instance)
            return;
        const auto vtable = *reinterpret_cast<void***>(instance);
        fn_ptr_           = vtable[index];
#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
        ptr_size_         = sizeof(Fn);
#endif
    }
};

class hook_entry
{
    void* target_ = nullptr;
    void* detour_ = nullptr; // [In] Address of the detour function.
#if 0                        // x64
    void* pRelay = nullptr; // [Out] Address of the relay function.
#endif
    bool patch_above_ = false; // [Out] Should use the hot patch area?
                               /// uint32_t_t ips_count   = 0;     // [Out] Number of the instruction boundaries.
#if 0
    std::vector<uint8_t> old_ips_; // [Out] Instruction boundaries of the target function.
    std::vector<uint8_t> new_ips_; // [Out] Instruction boundaries of the trampoline function.
#endif

    std::vector<uint8_t> trampoline_;
    fd::mem_protect trampoline_protection_;

    bool enabled_ = false;
    std::vector<uint8_t> target_backup_;

  public:
    hook_entry();
    ~hook_entry();

    hook_entry(hook_entry&& other);
    hook_entry& operator=(hook_entry&& other);

    bool create();
    bool created() const;
    bool enabled() const;

    bool enable();
    bool disable();

    void* get_original_method() const;
    void* get_target_method() const;
    void* get_replace_method() const;

    void set_target_method(void* getter);
    void set_replace_method(void* getter);
};

struct hook_impl : fd::hook_base
{
    ~hook_impl() override;
    hook_impl();

    bool enable() final;
    bool disable() final;

    bool initialized() const final;
    bool active() const final;

    void* get_original_method() const final;

  protected:
    void init(const function_getter target, const function_getter replace);

  private:
    hook_entry entry_;
};

template <class Impl>
concept have_callback_method = requires
{
    &Impl::callback;
};

template <class Impl>
concept have_static_callback_method = have_callback_method<Impl> && std::is_pointer_v<decltype(&Impl::callback)> && std::is_function_v<decltype(Impl::callback)>;

template <class Impl>
concept have_member_callback_method = have_callback_method<Impl> && std::is_member_function_pointer_v<decltype(&Impl::callback)>;

template <class Impl, size_t Index = 0>
struct hook_instance_static
{
    constexpr hook_instance_static()
    {
        static_assert(have_static_callback_method<Impl>, "Incorrect function type passed");
    }

  protected:
    template <typename... Args>
    static decltype(auto) call_original(Args&&... args)
    {
        auto fn                      = &Impl::callback;
        reinterpret_cast<void*&>(fn) = FD_OBJECT_GET(Impl, Index)->get_original_method();
        return std::invoke(fn, std::forward<Args>(args)...);
    }
};

template <class Impl, size_t Index = 0>
struct hook_instance_member
{
    constexpr hook_instance_member()
    {
        static_assert(have_member_callback_method<Impl>, "Incorrect function type passed");
    }

  protected:
    template <typename... Args>
    decltype(auto) call_original(Args&&... args) const
    {
        const auto inst    = &FD_OBJECT_GET(Impl, Index);
        const auto thisptr = static_cast<const Impl*>(this);
        FD_ASSERT(inst != thisptr, "Function must be called from hooked method!");
        const auto orig_fn = inst->get_original_method();

        auto def_callback = &Impl::callback;
        if constexpr (sizeof(decltype(def_callback)) == sizeof(void*))
        {
            reinterpret_cast<void*&>(def_callback) = orig_fn;
            return std::invoke(def_callback, thisptr, std::forward<Args>(args)...);
        }
        else
        {
            // avoid 'fat pointer' call
            using trivial_inst                      = decltype(_Tiny_selector(def_callback));
            auto tiny_callback                      = &trivial_inst::callback;
            reinterpret_cast<void*&>(tiny_callback) = orig_fn;
            return std::invoke(tiny_callback, reinterpret_cast<const trivial_inst*>(thisptr), std::forward<Args>(args)...);
        }
    }
};

export namespace fd
{
    using ::function_getter;

    using ::hook_impl;
    using ::hook_instance_member;
    using ::hook_instance_static;
} // namespace fd
