module;

export module fd.thread_pool;
export import fd.task;
export import fd.functional.fn;
export import fd.smart_ptr.shared;

using namespace fd;

struct basic_thread_pool
{
    using function_type = function<void() const>;
    using task_type     = shared_ptr<task>;

    struct simple_tag_t
    {
    };

    struct lazy_tag_t
    {
    };

    virtual ~basic_thread_pool() = default;

    virtual void wait() = 0;

    virtual bool operator()(function_type func, const simple_tag_t)    = 0;
    virtual task_type operator()(function_type func)                   = 0;
    virtual task_type operator()(function_type func, const lazy_tag_t) = 0;
};

export namespace fd
{
    using ::basic_thread_pool;

    basic_thread_pool* thread_pool;
} // namespace fd
