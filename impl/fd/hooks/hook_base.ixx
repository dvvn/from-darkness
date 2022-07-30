module;

export module fd.hook_base;
export import fd.string;

export namespace fd
{
    struct hook_base
    {
        virtual ~hook_base() = default;

        virtual bool enable()  = 0;
        virtual bool disable() = 0;

        virtual bool is_static() const   = 0;
        virtual string_view name() const = 0;

        virtual bool initialized() const = 0;
        virtual bool active() const      = 0;

        virtual void* get_original_method() const = 0;
        virtual void init()                       = 0;
    };
} // namespace fd
