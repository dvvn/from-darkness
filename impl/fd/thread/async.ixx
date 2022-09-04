module;

#include <fd/object.h>

export module fd.async;
export import fd.functional.fn;
export import fd.smart_ptr;

using namespace fd;

using function_type = function<void()>;

struct basic_task_data
{
    virtual ~basic_task_data() = default;

    virtual void on_start() = 0;
    virtual void on_wait()  = 0;
};

class task
{
    shared_ptr<basic_task_data> data_;

  public:
    template <class... T>
    task(T&&... args)
        : data_(std::forward<T>(args)...)
    {
    }

    basic_task_data* _Data() const;

    void start();
    void wait();
};

struct simple_tag_t
{
};

struct lazy_tag_t
{
};

struct basic_thread_pool
{
    virtual ~basic_thread_pool() = default;

    virtual void wait() = 0;

    virtual bool operator()(function_type func, const simple_tag_t) = 0;
    virtual task operator()(function_type func)                     = 0;
    virtual task operator()(function_type func, const lazy_tag_t)   = 0;
};

FD_OBJECT(async, basic_thread_pool);

export namespace fd
{
    using ::async;
    using async_task = task;

    namespace async_tags
    {
        constexpr simple_tag_t simple;
        constexpr lazy_tag_t lazy;
    } // namespace async_tags
} // namespace fd
