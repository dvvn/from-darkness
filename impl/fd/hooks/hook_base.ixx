module;

export module fd.hooks.base;
export import fd.string;

export namespace fd::hooks
{
    struct base
    {
        virtual ~base() = default;

        virtual bool enable()  = 0;
        virtual bool disable() = 0;

        virtual string_view name() const = 0;

        virtual bool initialized() const = 0;
        virtual bool active() const      = 0;

        virtual void* get_original_method() const = 0;
    };
} // namespace fd::hooks
