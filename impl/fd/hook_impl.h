#pragma once

#include <fd/functional.h>
#include <fd/hook.h>

#include <optional>
#include <vector>

namespace fd
{
    class function_getter
    {
        void* fnPtr_ = nullptr;

      public:
        operator void*() const
        {
            return fnPtr_;
        }

        void* get() const
        {
            return fnPtr_;
        }

        function_getter() = default;

        function_getter(void* ptr)
            : fnPtr_(ptr)
        {
        }

        template <class T>
        function_getter(T& ptr) requires(std::is_class_v<T> && !std::derived_from<T, function_getter>)
            : fnPtr_(static_cast<void*>(ptr))
        {
        }

        template <class T>
        function_getter(T obj) requires(std::is_member_function_pointer_v<T>)
            : fnPtr_(reinterpret_cast<void*&>(obj))
        {
        }

        function_getter(const ptrdiff_t addr)
            : fnPtr_(reinterpret_cast<void*>(addr))
        {
        }

#ifndef _MSC_VER
        template <class T>
        function_getter(T obj) requires(std::is_function_v<T>)
            : fnPtr_(reinterpret_cast<void*>(obj))
        {
        }
#endif

        function_getter(const function_getter hookInstance, const size_t index)
        {
            if (!hookInstance)
                return;
            const auto vtable = *static_cast<void***>(hookInstance.get());
            fnPtr_            = vtable[index];
        }

        function_getter(const function_getter& other)            = default;
        function_getter& operator=(const function_getter& other) = default;
    };

    struct hook_global_callback
    {
        virtual ~hook_global_callback() = default;

        virtual void construct(basic_hook* caller)                    = 0;
        virtual void destroy(const basic_hook* caller, bool unhooked) = 0;
    };

    extern hook_global_callback* HookGlobalCallback;

    class hook_impl : public basic_hook
    {
        bool inUse_;
        void* entry_;
        string_view name_;

      public:
        hook_impl();
        ~hook_impl() override;

        hook_impl(const hook_impl&) = delete;
        hook_impl(hook_impl&& other) noexcept;

        bool enable() override;
        bool disable() override;

        string_view name() const override;
        void set_name(string_view name);

        bool initialized() const override;
        bool active() const override;

        void* get_original_method() const;
        void init(function_getter target, function_getter replace);

        explicit operator bool() const;
    };

    template <class Ret>
    struct hook_callback_ret_wrapper : std::optional<Ret>
    {
    };

    template <>
    struct hook_callback_ret_wrapper<void>
    {
      private:
        bool value_ = false;

      public:
        operator bool() const;
        void emplace();
    };

    template <class Ret, class Orig, class Itr, typename... Args>
    static Ret _hook_callback(Orig original, Itr firstFn, Itr lastFn, Args... args) noexcept
    {
        // ReSharper disable CppUnreachableCode

        hook_callback_ret_wrapper<Ret> ret;
        auto interrupt = false;
        do
        {
            invoke(*firstFn, original, ret, interrupt, std::forward<Args>(args)...);
        }
        while (!interrupt && ++firstFn != lastFn);

        if (ret)
        {
            if constexpr (std::is_void_v<Ret>)
                return;
            else
                return *std::move(ret);
        }
        return invoke(original, std::forward<Args>(args)...);

        // ReSharper restore CppUnreachableCode
    }

    template <class Ret, typename... Args>
    struct basic_hook_callback : hook_impl
    {
        using original_wrapped  = function<Ret(Args...) const>;
        using ret_wrapped       = hook_callback_ret_wrapper<Ret>;
        using function_type     = function<void(const original_wrapped&, ret_wrapped&, bool&, Args...) const>;
        using callbacks_storage = std::vector<function_type>;

      protected:
        callbacks_storage callbacks;

      public:
        basic_hook_callback() = default;

        basic_hook_callback(const basic_hook_callback&)            = delete;
        basic_hook_callback& operator=(const basic_hook_callback&) = delete;

        void add(function_type fn)
        {
            callbacks.emplace_back(std::move(fn));
        }
    };

    template <class Ret, call_cvs Cvs, class Class, typename... Args>
    class hook_callback;

#define FD_HOOK_CALLBACK_MEMBER(_CCVS_)                                                                                    \
    template <class Ret, class Class, typename... Args>                                                                    \
    class hook_callback<Ret, call_cvs::_CCVS_##_, Class, Args...> : public basic_hook_callback<Ret, Class*, Args...>       \
    {                                                                                                                      \
        inline static hook_callback* self_;                                                                                \
        Ret __##_CCVS_ proxy(Args... args)                                                                                 \
        {                                                                                                                  \
            return _hook_callback<Ret>(                                                                                    \
                [hint = &hook_callback::proxy, orig = self_->get_original_method()](void* thisPtr, Args... args2) -> Ret { \
                    return invoke(hint, orig, static_cast<hook_callback*>(thisPtr), std::forward<Args>(args2)...);         \
                },                                                                                                         \
                self_->callbacks.begin(),                                                                                  \
                self_->callbacks.end(),                                                                                    \
                reinterpret_cast<Class*>(this),                                                                            \
                std::forward<Args>(args)...);                                                                              \
        }                                                                                                                  \
        void construct(void* target)                                                                                       \
        {                                                                                                                  \
            self_ = this;                                                                                                  \
            this->init(target, &hook_callback::proxy);                                                                     \
        }                                                                                                                  \
                                                                                                                           \
      public:                                                                                                              \
        hook_callback(function_getter target)                                                                              \
        {                                                                                                                  \
            construct(target);                                                                                             \
        }                                                                                                                  \
        hook_callback(Ret (__##_CCVS_ Class::*)(Args...), function_getter target)                                          \
        {                                                                                                                  \
            construct(target);                                                                                             \
        }                                                                                                                  \
    };                                                                                                                     \
    template <class Ret, class Class, typename... Args>                                                                    \
    hook_callback(Ret (__##_CCVS_ Class::*)(Args...)) -> hook_callback<Ret, call_cvs::_CCVS_##_, Class, Args...>;          \
    template <class Ret, class Class, typename... Args>                                                                    \
    hook_callback(Ret (__##_CCVS_ Class::*)(Args...), function_getter) -> hook_callback<Ret, call_cvs::_CCVS_##_, Class, Args...>;

#define FD_HOOK_CALLBACK(_CCVS_)                                                                                                   \
    template <class Ret, typename... Args>                                                                                         \
    class hook_callback<Ret, call_cvs::_CCVS_##_, void, Args...> : public basic_hook_callback<Ret, Args...>                        \
    {                                                                                                                              \
        inline static hook_callback* self_;                                                                                        \
        static Ret __##_CCVS_ proxy(Args... args)                                                                                  \
        {                                                                                                                          \
            return _hook_callback<Ret>(bind_front(Invoker, &hook_callback::proxy, self_->get_original_method()), /*-------------*/ \
                                       self_->callbacks.begin(),                                                                   \
                                       self_->callbacks.end(),                                                                     \
                                       std::forward<Args>(args)...);                                                               \
        }                                                                                                                          \
        void construct(void* target)                                                                                               \
        {                                                                                                                          \
            self_ = this;                                                                                                          \
            this->init(target, &hook_callback::proxy);                                                                             \
        }                                                                                                                          \
                                                                                                                                   \
      public:                                                                                                                      \
        hook_callback(function_getter target)                                                                                      \
        {                                                                                                                          \
            construct(target);                                                                                                     \
        }                                                                                                                          \
        hook_callback(Ret(__##_CCVS_*)(Args...), function_getter target)                                                           \
        {                                                                                                                          \
            construct(target);                                                                                                     \
        }                                                                                                                          \
    };                                                                                                                             \
    template <class Ret, typename... Args>                                                                                         \
    hook_callback(Ret(__##_CCVS_*)(Args...)) -> hook_callback<Ret, call_cvs::_CCVS_##_, void, Args...>;                            \
    template <class Ret, typename... Args>                                                                                         \
    hook_callback(Ret(__##_CCVS_*)(Args...), function_getter) -> hook_callback<Ret, call_cvs::_CCVS_##_, void, Args...>;

#define FD_HOOK_CALLBACK_ANY(_CCVS_) \
    FD_HOOK_CALLBACK_MEMBER(_CCVS_); \
    FD_HOOK_CALLBACK(_CCVS_);

#undef cdecl

    FD_HOOK_CALLBACK_ANY(cdecl);
    FD_HOOK_CALLBACK_ANY(fastcall);
    FD_HOOK_CALLBACK_ANY(stdcall);
    FD_HOOK_CALLBACK_ANY(vectorcall);
    FD_HOOK_CALLBACK_MEMBER(thiscall);

#undef FD_HOOK_CALLBACK_MEMBER
#undef FD_HOOK_CALLBACK
#undef FD_HOOK_CALLBACK_ANY

    template <typename T>
    using hook_callback_t = decltype(hook_callback(std::declval<T>()));
} // namespace fd