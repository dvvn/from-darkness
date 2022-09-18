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

    virtual ~basic_thread_pool() = default;

    virtual void wait() = 0;

    virtual bool add_simple(function_type func)    = 0;
    virtual task_type add(function_type func)      = 0;
    virtual task_type add_lazy(function_type func) = 0;
};

export namespace fd
{
    using ::basic_thread_pool;

    basic_thread_pool* thread_pool;
} // namespace fd
