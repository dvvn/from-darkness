module;

#include <fd/object.h>

export module fd.async;
export import fd.functional;

class custom_atomic_bool
{
    char value_;

  public:
    custom_atomic_bool(const bool value);

    operator bool() const;
    custom_atomic_bool& operator=(const bool value);
};

using fd::function;

using function_type    = function<void()>;
using function_type_ex = function<void(const custom_atomic_bool&)>;

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
    using ::task;
} // namespace fd
