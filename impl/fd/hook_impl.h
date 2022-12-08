#pragma once

#include <fd/functional.h>
#include <fd/hook.h>

// #define FUNCTION_GETTER_HAVE_PTR_SIZE

namespace fd
{
    class function_getter
    {
        void* fnPtr_ = nullptr;
#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
        uint8_t ptr_size_ = 0;
#endif

      public:
        operator void*() const
        {
            return fnPtr_;
        }

#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
        uint8_t size() const
        {
            return ptr_size_;
        }
#endif

        void* get() const
        {
            return fnPtr_;
        }

        function_getter() = default;

        template <typename Fn>
        function_getter(Fn fn)
        {
            fnPtr_ = reinterpret_cast<void*&>(fn);
#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
            ptr_size_ = sizeof(fn);
#endif
        }

        template <class C, class Fn = void*>
        function_getter(C* hookInstance, const size_t index, Fn = {})
        {
            if (!hookInstance)
                return;
            const auto vtable = *reinterpret_cast<void***>(hookInstance);
            fnPtr_           = vtable[index];
#ifdef FUNCTION_GETTER_HAVE_PTR_SIZE
            ptr_size_ = sizeof(Fn);
#endif
        }
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