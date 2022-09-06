module;

export module fd.hooks_loader;
export import fd.hook_base;

export namespace fd
{
    struct hooks_loader
    {
        virtual ~hooks_loader() = default;

        virtual void disable()                               = 0;
        virtual bool enable(const bool stop_on_error = true) = 0;
        virtual void store(hook_base* const hook)            = 0;
    };
} // namespace fd
