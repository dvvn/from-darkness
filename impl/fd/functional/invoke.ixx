module;

#include <fd/utility.h>

#include <concepts>
#include <functional>

export module fd.functional.invoke;
enum class call_cvs : uint8_t
{
    default__,
    thiscall__,
    cdecl__,
    fastcall__,
    stdcall__,
    vectorcall__,
};

template <call_cvs, typename Ret, typename... Args>
struct tiny_helper;

#define _CALL_CVS_IMPL(_ENUM_, _CCVS_)                                                                       \
    template <typename Ret, typename... Args>                                                                \
    struct tiny_helper<call_cvs::_ENUM_, Ret, Args...>                                                       \
    {                                                                                                        \
        Ret _CCVS_ callback(Args... args) const                                                              \
        {                                                                                                    \
            if constexpr (!std::is_void_v<Ret>)                                                              \
                return *(Ret*)nullptr;                                                                       \
        }                                                                                                    \
    };                                                                                                       \
    template <typename Ret, class T, typename... Args>                                                       \
    constexpr tiny_helper<call_cvs::_ENUM_, Ret, Args...> _Tiny_selector(Ret (_CCVS_ T::*fn)(Args...))       \
    {                                                                                                        \
        return {};                                                                                           \
    }                                                                                                        \
    template <typename Ret, class T, typename... Args>                                                       \
    constexpr tiny_helper<call_cvs::_ENUM_, Ret, Args...> _Tiny_selector(Ret (_CCVS_ T::*fn)(Args...) const) \
    {                                                                                                        \
        return {};                                                                                           \
    }

#define _CALL_CVS(_C_) _CALL_CVS_IMPL(_C_##__, __##_C_)

#ifdef _WIN32
FOR_EACH(_CALL_CVS, thiscall, cdecl, fastcall, stdcall, vectorcall);
#else
_CALL_CVS(default, );
#endif

template <typename Fn>
concept fat_pointer = sizeof(Fn) > sizeof(void*);

template <typename Fn>
concept tiny_pointer = sizeof(Fn) == sizeof(void*);

template <typename T>
concept void_ptr = std::same_as<T, void*>;

export namespace fd
{
    using std::invoke;

    template <fat_pointer Fn, typename... Args>
    decltype(auto) invoke(Fn fn, void_ptr auto actual_fn, auto* thisptr, Args&&... args)
    {
        using trivial_inst = decltype(_Tiny_selector(fn));

        union
        {
            decltype(&trivial_inst::callback) tiny;
            Fn hint;
            void* raw;
        } adaptor;

        adaptor.hint = fn;
        adaptor.raw  = actual_fn;

        return invoke(adaptor.tiny, reinterpret_cast<const trivial_inst*>(thisptr), std::forward<Args>(args)...);
    }

    template <tiny_pointer Fn, typename... Args>
    decltype(auto) invoke(Fn fn, void_ptr auto actual_fn, Args&&... args) requires(sizeof...(Args) > 0 || !std::invocable<Fn, void*>)
    {
        union
        {
            Fn fake;
            void* raw;
        } adaptor;

        adaptor.raw = actual_fn;
        return invoke(adaptor.fake, std::forward<Args>(args)...);
    }

    template <typename Fn, typename... Args>
    decltype(auto) invoke(Fn fn, const std::integral auto index, auto* thisptr, Args&&... args)
    {
        auto vtable = *(void***)thisptr;
        auto vfunc  = vtable[index];
        return invoke(fn, vfunc, thisptr, std::forward<Args>(args)...);
    }

    template <typename... Args>
    concept invocable = requires(Args&&... args)
    {
        invoke(std::forward<Args>(args)...);
    };

    template <typename... Args>
    using invoke_result = decltype(invoke(std::declval<Args>()...));

} // namespace fd
