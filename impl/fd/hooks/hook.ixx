module;

#include <fd/assert.h>
#include <fd/object.h>

#include <concepts>
#include <vector>

export module fd.hook;
export import fd.hook_base;
import fd.memory;

//#define FUNCTION_GETTER_HAVE_PTR_SIZE

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

    struct hook_impl : hook_base
    {
        ~hook_impl() override;
        hook_impl();

        bool enable() final;
        bool disable() final;

        bool initialized() const final;
        bool active() const final;

        void* get_original_method() const final;

      protected:
        void init(const function_getter target, const function_getter replace);

      private:
        unique_ptr<hook_entry> entry_;
    };

} // namespace fd

template <class Impl>
concept have_callback_method = requires
{
    &Impl::callback;
};

template <class Impl>
concept have_static_callback_method = have_callback_method<Impl> && std::is_pointer_v<decltype(&Impl::callback)> && std::is_function_v<decltype(Impl::callback)>;

template <class Impl>
concept have_member_callback_method = have_callback_method<Impl> && std::is_member_function_pointer_v<decltype(&Impl::callback)>;

template <class Impl, size_t Index>
auto _Get_original_method()
{
    using callback_t = decltype(&Impl::callback);

    union
    {
        callback_t fn;
        const void* ptr;
    } adaptor;

    if constexpr (sizeof(callback_t) > sizeof(void*))
        adaptor.fn = nullptr; // override 'fat' pointer if exists
    adaptor.ptr = FD_OBJECT_GET(Impl, Index)->get_original_method();
    return adaptor.fn;
}

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
            return invoke(_Get_original_method<Impl, Index>(), std::forward<Args>(args)...);
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
            FD_ASSERT((&FD_OBJECT_GET(Impl, Index) != thisptr), "Function must be called from hooked method!");
            return invoke(_Get_original_method<Impl, Index>(), thisptr, std::forward<Args>(args)...);
        }
    };
} // namespace fd
