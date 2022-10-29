module;

#include <cstdint>

export module fd.hooks.impl;
export import fd.hooks.base;

// #define FUNCTION_GETTER_HAVE_PTR_SIZE

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
            void* entry_;
            string_view name_;

          protected:
            void init(const function_getter target, const function_getter replace);

          public:
            ~impl() override;
            impl(const string_view name = {});

            impl(const impl&) = delete;
            impl(impl&& other);

            bool enable() override;
            bool disable() override;

            string_view name() const override;

            bool initialized() const override;
            bool active() const override;

            void* get_original_method() const override;

            explicit operator bool() const;
        };
    } // namespace hooks

} // namespace fd
