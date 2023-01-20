#pragma once

#include <fd/hook.h>



namespace fd
{
    class hook_impl : public basic_hook
    {
        bool        inUse_;
        void*       entry_;
        string_view name_;

      public:
        hook_impl();
        ~hook_impl() override;

        hook_impl(const hook_impl&) = delete;
        hook_impl(hook_impl&& other) noexcept;

        bool enable() override;
        bool disable() override;

        string_view name() const override;
        void        set_name(string_view name);

        bool initialized() const override;
        bool active() const override;

        void* get_original_method() const;
        void  init(void* target, void* replace);

        explicit operator bool() const;
    };

    
    

} // namespace fd