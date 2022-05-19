module;

#include <cheat/tools/interface.h>

#include <nstd/runtime_assert.h>

#include <concepts>
#include <functional>
#include <string>

export module cheat.hooks.hook;
export import cheat.hooks.base;
import dhooks.entry;

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

#define TINY_HELPER(_C_)                                                                                                                                                           \
    template <typename Ret, typename... Args>                                                                                                                                      \
    struct tiny_helper<call_cvs::_C_##__, Ret, Args...>                                                                                                                            \
    {                                                                                                                                                                              \
        Ret __##_C_ callback(Args... args) const noexcept                                                                                                                          \
        {                                                                                                                                                                          \
            if constexpr (!std::is_void_v<Ret>)                                                                                                                                    \
                return *(Ret*)nullptr;                                                                                                                                             \
        }                                                                                                                                                                          \
    };

#define TINY_SELECTOR(_C_)                                                                                                                                                         \
    template <typename Ret, class T, typename... Args>                                                                                                                             \
    constexpr tiny_helper<call_cvs::_C_##__, Ret, Args...> _Tiny_selector(Ret (__##_C_ T::*fn)(Args...))                                                                           \
    {                                                                                                                                                                              \
        return {};                                                                                                                                                                 \
    }                                                                                                                                                                              \
    template <typename Ret, class T, typename... Args>                                                                                                                             \
    constexpr tiny_helper<call_cvs::_C_##__, Ret, Args...> _Tiny_selector(Ret (__##_C_ T::*fn)(Args...) const)                                                                     \
    {                                                                                                                                                                              \
        return {};                                                                                                                                                                 \
    }

#define TINY_IMPL(_C_) TINY_HELPER(_C_) TINY_SELECTOR(_C_)

TINY_IMPL(thiscall);
TINY_IMPL(cdecl);
TINY_IMPL(fastcall);
TINY_IMPL(stdcall);
TINY_IMPL(vectorcall);

class function_getter
{
    void* fn_ptr_;
    uint8_t ptr_size_;

  public:
    operator void*() const noexcept
    {
        return fn_ptr_;
    }

    uint8_t size() const noexcept
    {
        return ptr_size_;
    }

    void* get() const noexcept
    {
        return fn_ptr_;
    }

    function_getter()
    {
        fn_ptr_ = nullptr;
        ptr_size_ = 0;
    }

    template <typename Fn>
    function_getter(Fn fn)
    {
        fn_ptr_ = reinterpret_cast<void*&>(fn);
        ptr_size_ = sizeof(fn);
    }

    template <class C, class Fn = void*>
    function_getter(C* instance, const size_t index, Fn = {})
    {
        const auto vtable = *reinterpret_cast<void***>(instance);
        fn_ptr_ = vtable[index];
        ptr_size_ = sizeof(Fn);
    }
};

export namespace cheat::hooks
{
    class hook : public virtual base
    {
      public:
        using entry_type = dhooks::hook_entry;

        ~hook() override;

        bool enable() runtime_assert_noexcept override;
        bool disable() runtime_assert_noexcept override;

        void* get_original_method() const runtime_assert_noexcept;

      protected:
      void init(const function_getter target, const function_getter replace) runtime_assert_noexcept
      {
          entry_.set_target_method(target);
          entry_.set_replace_method(replace);
      }

      private:
        entry_type entry_;
    };

    template <class Impl, class Inst = one_instance<Impl>>
    struct hook_instance_static : Inst
    {
        constexpr hook_instance_static()
        {
            static_assert(!std::is_member_function_pointer_v<decltype(&Impl::callback)> && std::derived_from<Impl, static_base>, "Incorrect function type passed");
        }

      protected:
        template <typename... Args>
        static decltype(auto) call_original(Args&&... args) runtime_assert_noexcept
        {
            auto fn = &Impl::callback;
            reinterpret_cast<void*&>(fn) = Inst::get().get_original_method();
            return std::invoke(fn, std::forward<Args>(args)...);
        }
    };

    template <class Impl, class Inst = one_instance<Impl>>
    struct hook_instance_member : Inst
    {
        constexpr hook_instance_member()
        {
            static_assert(std::is_member_function_pointer_v<decltype(&Impl::callback)> && std::derived_from<Impl, class_base>, "Incorrect function type passed");
        }

      protected:
        template <typename... Args>
        decltype(auto) call_original(Args&&... args) const runtime_assert_noexcept
        {
            const auto inst = Inst::get_ptr();
            runtime_assert(inst != this, "Function must be called from hooked method!");
            const auto thisptr = static_cast<const Impl*>(this);
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
                using trivial_inst = decltype(_Tiny_selector(def_callback));
                auto tiny_callback = &trivial_inst::callback;
                reinterpret_cast<void*&>(tiny_callback) = orig_fn;
                return std::invoke(tiny_callback, reinterpret_cast<const trivial_inst*>(thisptr), std::forward<Args>(args)...);
            }
        }
    };

    /*template<class Impl>
    struct hook_instance :std::conditional_t<std::derived_from<Impl, class_base>, hook_instance_member<Impl>, hook_instance_static<Impl>>
    {
    };*/
} // namespace cheat::hooks
