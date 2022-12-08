#pragma once

#include <fd/utility.h>

#include <function2/function2.hpp>

#include <functional>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace fd
{
#if !defined(__cpp_lib_bind_front) || !defined(__cpp_lib_bind_back)
    enum class _Bind_mode : uint8_t
    {
        front,
        back
    };

    template <_Bind_mode Mode>
    struct _Bind_caller;

    template <_Bind_mode Mode>
    struct _Bind_impl
    {
        template <typename Fn, typename... Args>
        constexpr decltype(auto) operator()(Fn&& fn, Args&&... args) const
        {
            return [_Fn   = std::forward<Fn>(fn), //
                    _Args = std::tuple(std::forward<Args>(args)...)]<typename... CallArgs>(CallArgs&&... call_args) -> decltype(auto) {
                if constexpr (sizeof...(Args) == 0)
                    return invoke(_Fn, std::forward<CallArgs>(call_args)...);
                else
                    return _Bind_caller<Mode>::call(_Fn, _Args, std::forward<CallArgs>(call_args)...);
            };
        }
    };
#endif

#ifdef __cpp_lib_bind_front
    using std::bind_front;
#else
    template <>
    struct _Bind_caller<_Bind_mode::front>
    {
        template <typename Fn, class Tpl, typename... Args>
        static constexpr decltype(auto) call(Fn& fn, Tpl& args, Args&&... call_args)
        {
            return std::apply(
                [&](auto&... bound_args) -> decltype(auto) {
                    return invoke(fn, bound_args..., std::forward<Args>(call_args)...);
                },
                args);
        }
    };

    constexpr _Bind_impl<_Bind_mode::front> bind_front;
#endif

#ifdef __cpp_lib_bind_back
    using std::bind_back;
#else
    template <>
    struct _Bind_caller<_Bind_mode::back>
    {
        template <typename Fn, class Tpl, typename... Args>
        static constexpr decltype(auto) call(Fn& fn, Tpl& args, Args&&... call_args)
        {
            return std::apply(
                [&](auto&... bound_args) -> decltype(auto) {
                    return invoke(fn, std::forward<Args>(call_args)..., bound_args...);
                },
                args);
        }
    };

    constexpr _Bind_impl<_Bind_mode::back> bind_back;
#endif

    //-------------

    enum class _Call_cvs : uint8_t
    {
        // ReSharper disable CppInconsistentNaming
        default__,
        thiscall__,
        cdecl__,
        fastcall__,
        stdcall__,
        vectorcall__,
        // ReSharper restore CppInconsistentNaming
    };

    template <_Call_cvs, typename Ret, typename... Args>
    struct _Tiny_fn;

#define _CALL_CVS_IMPL(_ENUM_, _CCVS_)                                                                     \
    template <typename Ret, typename... Args>                                                              \
    struct _Tiny_fn<_Call_cvs::_ENUM_, Ret, Args...>                                                       \
    {                                                                                                      \
        Ret _CCVS_ callback(Args... args) const                                                            \
        {                                                                                                  \
            if constexpr (!std::is_void_v<Ret>)                                                            \
                return *(Ret*)nullptr;                                                                     \
        }                                                                                                  \
    };                                                                                                     \
    template <typename Ret, class T, typename... Args>                                                     \
    constexpr _Tiny_fn<_Call_cvs::_ENUM_, Ret, Args...> _Tiny_selector(Ret (_CCVS_ T::*fn)(Args...))       \
    {                                                                                                      \
        return {};                                                                                         \
    }                                                                                                      \
    template <typename Ret, class T, typename... Args>                                                     \
    constexpr _Tiny_fn<_Call_cvs::_ENUM_, Ret, Args...> _Tiny_selector(Ret (_CCVS_ T::*fn)(Args...) const) \
    {                                                                                                      \
        return {};                                                                                         \
    }

#define _CALL_CVS(_C_) _CALL_CVS_IMPL(FD_CONCAT(_C_, __), FD_CONCAT(__, _C_))

#ifdef _WIN32
#ifndef __RESHARPER__
#undef cdecl
    FOR_EACH(_CALL_CVS, thiscall, cdecl, fastcall, stdcall, vectorcall);
#endif
#else
    _CALL_CVS(default, );
#endif

#undef _CALL_CVS

    template <typename Fn>
    concept fat_pointer = sizeof(Fn) > sizeof(void*);

    template <typename Fn>
    concept tiny_pointer = sizeof(Fn) == sizeof(void*);

    template <typename T>
    concept void_pointer = std::is_pointer_v<T> && std::is_void_v<std::remove_pointer_t<T>>;

    using std::invoke;

    template <fat_pointer Fn, void_pointer ActFn, typename... Args>
    decltype(auto) invoke(Fn fn, ActFn actualFn, auto* thisptr, Args&&... args)
    {
        using trivial_inst = decltype(_Tiny_selector(fn));

        union
        {
            decltype(&trivial_inst::callback) tiny;
            Fn hint;
            ActFn raw;
        } adaptor;

        adaptor.hint = fn;
        adaptor.raw  = actualFn;

        return invoke(adaptor.tiny, reinterpret_cast<const trivial_inst*>(thisptr), std::forward<Args>(args)...);
    }

    template <tiny_pointer Fn, void_pointer ActFn, typename... Args>
    decltype(auto) invoke(Fn fn, ActFn actualFn, Args&&... args) requires(!std::invocable<Fn, ActFn, Args && ...> && std::invocable<Fn &&, Args && ...>)
    {
        union
        {
            Fn fake;
            ActFn raw;
        } adaptor;

        adaptor.raw = actualFn;
        return invoke(adaptor.fake, std::forward<Args>(args)...);
    }

    template <typename Fn, typename... Args>
    decltype(auto) invoke(Fn fn, const std::integral auto index, auto* thisptr, Args&&... args)
    {
        union
        {
            Fn hint;
            void*** vtablePtr;
        } adaptor;

        adaptor.hint = fn;

        auto vtable = *adaptor.vtablePtr;
        auto vfunc  = vtable[index];
        return invoke(fn, vfunc, thisptr, std::forward<Args>(args)...);
    }

    template <typename... Args>
    concept invocable = requires(Args&&... args) { invoke(std::forward<Args>(args)...); };

    template <typename... Args>
    using invoke_result = decltype(invoke(std::declval<Args>()...));

    constexpr auto Invoker = []<typename... Args>(Args&&... args) -> decltype(auto) {
        return invoke(std::forward<Args>(args)...);
    };

    //--------------

    template <typename T>
    concept can_be_null = requires(T obj) {
                              static_cast<bool>(obj);
                              obj = nullptr;
                          };

    template <typename Fn, bool = can_be_null<Fn>>
    struct lazy_invoke;

    template <typename Fn>
    class lazy_invoke_base
    {
        Fn fn_;

        friend struct lazy_invoke<Fn>;

      public:
        template <typename Fn1>
        constexpr lazy_invoke_base(Fn1&& fn)
            : fn_(std::forward<Fn1>(fn))
        {
        }

        lazy_invoke_base(const lazy_invoke_base&)            = delete;
        lazy_invoke_base& operator=(const lazy_invoke_base&) = delete;
    };

    template <typename Fn>
    struct lazy_invoke<Fn, true> : lazy_invoke_base<Fn>
    {
        constexpr ~lazy_invoke()
        {
            if (this->fn_)
                invoke(this->fn_);
        }

        using lazy_invoke_base<Fn>::lazy_invoke_base;
        using lazy_invoke_base<Fn>::operator=;

        constexpr void reset()
        {
            this->fn_ = nullptr;
        }
    };

    template <typename Fn>
    struct lazy_invoke<Fn, false> : lazy_invoke_base<Fn>
    {
      private:
        bool valid_ = true;

      public:
        constexpr ~lazy_invoke()
        {
            if (valid_)
                invoke(this->fn_);
        }

        using lazy_invoke_base<Fn>::lazy_invoke_base;
        using lazy_invoke_base<Fn>::operator=;

        constexpr void reset()
        {
            valid_ = false;
        }
    };

    template <typename Fn>
    lazy_invoke(Fn&&) -> lazy_invoke<std::decay_t<Fn>>;

    //--------------

    using fu2::function;
    using fu2::function_view;
    using fu2::unique_function;

    using fu2::detail::overloading::overload;
    using fu2::detail::overloading::overload_impl;
} // namespace fd
