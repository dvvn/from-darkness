module;

#include <fd/object.h>

#include <stop_token>

export module fd.async;
export import fd.functional;

using fd::function;
using std::stop_token;

using function_type    = function<void()>;
using function_type_ex = function<void(const stop_token&)>;

using task = void*; // WIP

struct lazy_tag_t
{
};

struct basic_thread_pool
{
    virtual ~basic_thread_pool() = default;

    virtual void wait() = 0;

    virtual task operator()(function_type func)                      = 0;
    virtual task operator()(function_type_ex func)                   = 0;
    virtual task operator()(function_type func, const lazy_tag_t)    = 0;
    virtual task operator()(function_type_ex func, const lazy_tag_t) = 0;
};

FD_OBJECT(async, basic_thread_pool);

export namespace fd
{
    using ::async;
    using ::stop_token;
    using ::task;
} // namespace fd
