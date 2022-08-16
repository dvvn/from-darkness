module;

#include <fd/utility.h>

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

#define _CALL_CVS(_C_)                                                                                         \
    template <typename Ret, typename... Args>                                                                  \
    struct tiny_helper<call_cvs::_C_##__, Ret, Args...>                                                        \
    {                                                                                                          \
        Ret __##_C_ callback(Args... args) const                                                               \
        {                                                                                                      \
            if constexpr (!std::is_void_v<Ret>)                                                                \
                return *(Ret*)nullptr;                                                                         \
        }                                                                                                      \
    };                                                                                                         \
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

FOR_EACH(_CALL_CVS, thiscall, cdecl, fastcall, stdcall, vectorcall);

template <typename Fn>
concept fat_pointer = sizeof(Fn) > sizeof(void*) && requires(Fn fn)
{
    _Tiny_selector(fn);
};

export namespace fd
{
    using std::invoke;

    template <fat_pointer Fn, class C, typename... Args>
    decltype(auto) invoke(Fn fn, C* thisptr, Args&&... args)
    {
        // avoid 'fat pointer' call
        using trivial_inst = decltype(_Tiny_selector(fn));

        union
        {
            decltype(&trivial_inst::callback) tiny;
            Fn fat;
        } adaptor;

        adaptor.fat = fn;
        return invoke(adaptor.tiny, reinterpret_cast<const trivial_inst*>(thisptr), std::forward<Args>(args)...);
    }

    template <typename Fn, class C, typename... Args>
    decltype(auto) invoke(Fn, const size_t index, C* thisptr, Args&&... args)
    {
        using cast_t = std::conditional_t<std::is_const_v<C>, const void, void>;
        auto vtable  = *reinterpret_cast<cast_t***>(thisptr);
        auto vfunc   = vtable[index];
        return invoke(reinterpret_cast<Fn&>(vfunc), thisptr, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    concept invocable = requires(T&& obj, Args&&... args)
    {
        invoke(std::forward<T>(obj), std::forward<Args>(args)...);
    };

} // namespace fd
