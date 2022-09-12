module;

#include <concepts>

export module fd.hooks.impl;
export import fd.hooks.base;
import fd.smart_ptr.unique;

// #define FUNCTION_GETTER_HAVE_PTR_SIZE

class hook_entry;

export namespace fd
{
    class function_getter
    {
        void* fn_ptr_ = nullptr;
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
            fn_ptr_ = reinterpret_cast<void*&>(fn);
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
            ptr_size_ = sizeof(Fn);
#endif
        }
    };

    namespace hooks
    {
        class impl : public base
        {
            unique_ptr<hook_entry> entry_;

          protected:
            void init(const function_getter target, const function_getter replace);

          public:
            ~impl() override;
            impl();

            impl(impl&&);

            bool enable() final;
            bool disable() final;

            bool initialized() const final;
            bool active() const final;

            void* get_original_method() const final;

            explicit operator bool() const;
        };
    } // namespace hooks

} // namespace fd

#if 0

template <class Impl>
concept have_callback_method = requires { &Impl::callback; };

template <class Impl>
concept have_static_callback_method = have_callback_method<Impl> && std::is_pointer_v<decltype(&Impl::callback)> && std::is_function_v<decltype(Impl::callback)>;

template <class Impl>
concept have_member_callback_method = have_callback_method<Impl> && std::is_member_function_pointer_v<decltype(&Impl::callback)>;

export namespace fd
{
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
            return invoke(&Impl::callback, FD_OBJECT_GET(Impl, Index)->get_original_method(), std::forward<Args>(args)...);
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
            const auto thisptr = static_cast<const Impl*>(this);
            const auto impl    = &FD_OBJECT_GET(Impl, Index);
            FD_ASSERT(impl != thisptr, "Function must be called from hooked method!");
            return invoke(&Impl::callback, impl->get_original_method(), const_cast<Impl*>(thisptr), std::forward<Args>(args)...);
        }
    };
} // namespace fd

#endif
