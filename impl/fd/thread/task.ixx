module;

export module fd.task;

struct task
{
    virtual ~task()      = default;
    virtual void start() = 0;
    virtual void wait()  = 0;
};

struct finished_task : task
{
    void start() override;
    void wait() override;
};

export namespace fd
{
    using ::finished_task;
    using ::task;
} // namespace fd
