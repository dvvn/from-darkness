module;

export module fd.async;
export import fd.task;
export import fd.functional.fn;

using namespace fd;

using function_type = function<void()>;

struct simple_tag_t
{
};

struct lazy_tag_t
{
};

struct async
{
    virtual ~async() = default;

    virtual void wait() = 0;

    virtual bool operator()(function_type func, const simple_tag_t) = 0;
    virtual task operator()(function_type func)                     = 0;
    virtual task operator()(function_type func, const lazy_tag_t)   = 0;
};

export namespace fd
{
    using ::async;

    namespace async_tags
    {
        constexpr simple_tag_t simple;
        constexpr lazy_tag_t lazy;
    } // namespace async_tags
} // namespace fd
