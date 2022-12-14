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

    class hook_impl : public hook
    {
        void* entry_;
        string_view name_;

      public:
        ~hook_impl() override;
        hook_impl(string_view name);

        hook_impl(const hook_impl&) = delete;
        hook_impl(hook_impl&& other) noexcept;

        bool enable() override;
        bool disable() override;

        string_view name() const override;

        bool initialized() const override;
        bool active() const override;

        void* get_original_method() const;
        void init(function_getter target, function_getter replace);

        explicit operator bool() const;
    };

    template <class T>
    struct hook_instance
    {
      protected:
        inline static T* self;

        template <typename... Args>
        decltype(auto) call_original(Args&&... args)
        {
            return invoke(&T::callback, self->get_original_method(), static_cast<T*>(this), std::forward<Args>(args)...);
        }

        template <typename... Args>
        static decltype(auto) call_original(Args&&... args) requires(!std::is_member_function_pointer_v<decltype(&T::callback)>)
        {
            return invoke(&T::callback, self->get_original_method(), std::forward<Args>(args)...);
        }

      public:
        hook_instance(function_getter target)
        {
            self = static_cast<T*>(this);
            self->init(target, &T::callback);
        }

        hook_instance(const hook_instance&)            = delete;
        hook_instance& operator=(const hook_instance&) = delete;
        hook_instance& operator=(hook_instance&&)      = delete;

        hook_instance(hook_instance&&) noexcept
        {
            self = static_cast<T*>(this);
        }
    };

    //-----------

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
        operator bool&();
        operator bool() const;
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
        using original_wrapped  = function_view<Ret(Args...) const>;
        using ret_wrapped       = hook_callback_ret_wrapper<Ret>;
        using function_type     = function_view<void(const original_wrapped&, ret_wrapped&, bool&, Args...) const>;
        using callbacks_storage = std::vector<function_type>;

      protected:
        callbacks_storage callbacks;

      public:
        basic_hook_callback(const basic_hook_callback&)            = delete;
        basic_hook_callback& operator=(const basic_hook_callback&) = delete;

        basic_hook_callback(const function_getter target, const function_getter replace, const string_view name)
            : hook_impl(name)
        {
            this->init(target, replace);
        }

        void add(function_type fn)
        {
            callbacks.emplace_back(std::move(fn));
        }
    };

    template <class Ret, call_cvs Cvs, class Class, typename... Args>
    class hook_callback;

#define HOOK_CALLBACK(_CCVS_)                                                                                                      \
    template <class Ret, class Class, typename... Args>                                                                            \
    class hook_callback<Ret, call_cvs::_CCVS_##_, Class, Args...> : public basic_hook_callback<Ret, Class*, Args...>               \
    {                                                                                                                              \
        inline static hook_callback* self_;                                                                                        \
        Ret __##_CCVS_ proxy(Args... args)                                                                                         \
        {                                                                                                                          \
            return _hook_callback<Ret>(                                                                                            \
                [hint = &hook_callback::proxy, orig = self_->get_original_method()](void* thisPtr, Args... args2) -> Ret {         \
                    return invoke(hint, orig, static_cast<hook_callback*>(thisPtr), std::forward<Args>(args2)...);                 \
                },                                                                                                                 \
                self_->callbacks.begin(),                                                                                          \
                self_->callbacks.end(),                                                                                            \
                reinterpret_cast<Class*>(this),                                                                                    \
                std::forward<Args>(args)...);                                                                                      \
        }                                                                                                                          \
                                                                                                                                   \
      public:                                                                                                                      \
        hook_callback(function_getter target, const string_view name = {})                                                         \
            : basic_hook_callback<Ret, Class*, Args...>(target, &hook_callback::proxy, name)                                       \
        {                                                                                                                          \
            self_ = this;                                                                                                          \
        }                                                                                                                          \
    };                                                                                                                             \
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
                                                                                                                                   \
      public:                                                                                                                      \
        hook_callback(function_getter target, const string_view name = {})                                                         \
            : basic_hook_callback<Ret, Args...>(target, &hook_callback::proxy, name)                                               \
        {                                                                                                                          \
            self_ = this;                                                                                                          \
        }                                                                                                                          \
    };

    HOOK_CALLBACK(cdecl);
    HOOK_CALLBACK(fastcall);
    HOOK_CALLBACK(stdcall);
    HOOK_CALLBACK(vectorcall);
} // namespace fd