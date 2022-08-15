module;

#include <function2/function2.hpp>

#include <concepts>
#include <functional>

export module fd.functional.invoke;

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

export namespace fd
{
    using fu2::detail::invocation::invoke;
    using std::invoke;

    template <typename T, typename... Args>
    concept invocable = requires(T&& obj, Args&&... args)
    {
        invoke(std::forward<T>(obj), std::forward<Args>(args)...);
    };

    template <typename Fn, class C, typename... Args>
    decltype(auto) invoke_member(Fn fn, C* thisptr, Args&&... args)
    {
        if constexpr (sizeof(Fn) == sizeof(void*))
        {
            return invoke(fn, thisptr, std::forward<Args>(args)...);
        }
        else
        {
            // avoid 'fat pointer' call
            using trivial_inst                   = decltype(_Tiny_selector(fn));
            auto tiny_callback                   = &trivial_inst::callback;
            reinterpret_cast<Fn&>(tiny_callback) = fn;
            return invoke(tiny_callback, reinterpret_cast<const trivial_inst*>(thisptr), std::forward<Args>(args)...);
        }
    }

    template <typename Fn, class C, typename... Args>
    decltype(auto) invoke_vfunc(Fn, const size_t index, C* thisptr, Args&&... args)
    {
        using cast_t = std::conditional_t<std::is_const_v<C>, const void, void>***;
        auto vtable  = *reinterpret_cast<cast_t>(thisptr);
        auto vfunc   = vtable[index];
        return invoke_member(reinterpret_cast<Fn&>(vfunc), thisptr, std::forward<Args>(args)...);
    }
} // namespace fd
