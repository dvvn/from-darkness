#pragma once

#include <fd/functional.h>
#include <fd/hook.h>

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
        hook_impl(string_view name = {});

        hook_impl(const hook_impl&) = delete;
        hook_impl(hook_impl&& other) noexcept;

        bool enable() override;
        bool disable() override;

        string_view name() const override;

        bool initialized() const override;
        bool active() const override;

        void* get_original_method() const override;

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
} // namespace fd
