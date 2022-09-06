module;

export module fd.task;
export import fd.smart_ptr.shared;

struct basic_task
{
    virtual ~basic_task() = default;

    virtual void start() = 0;
    virtual void wait()  = 0;
};

struct basic_finished_task final : basic_task
{
    void start() override;
    void wait() override;
};

export namespace fd
{
    using ::basic_task;
    using task = shared_ptr<basic_task>;

    struct finished_task : task
    {
        finished_task();
    };

} // namespace fd
