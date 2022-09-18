module;

export module fd.callback.invoker;
export import fd.functional.invoke;

export namespace fd
{
    template <typename... Args>
    struct basic_callback_invoker
    {
        virtual ~basic_callback_invoker()           = default;
        virtual void operator()(Args... args) const = 0;
    };

    template <class C, typename... Args>
    class callback_invoker : public basic_callback_invoker<Args...>
    {
        C* owner_;

      public:
        callback_invoker(C* owner)
            : owner_(owner)
        {
        }

        void operator()(Args... args) const override
        {
            invoke(*owner_, static_cast<Args>(args)...);
        }
    };

    template <class C, typename... Args>
    class callback_invoker<const C, Args...> : public basic_callback_invoker<Args...>
    {
        const C* owner_;

      public:
        callback_invoker(const C* owner)
            : owner_(owner)
        {
        }

        void operator()(Args... args) const override
        {
            invoke(*owner_, static_cast<Args>(args)...);
        }
    };

} // namespace fd
