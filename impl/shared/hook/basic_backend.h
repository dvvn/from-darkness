#pragma once

#define FD_ALLOCATE_HOOK_BACKEND

namespace fd
{
struct prepared_hook_data
{
    void *target;
    void *replace;
    void **original;
};

class basic_hook_backend
{
#ifdef FD_ALLOCATE_HOOK_BACKEND
  public:
    virtual ~basic_hook_backend() = default;
#else
  protected:
    ~basic_hook_backend() = default;
#endif

#ifndef FD_ALLOCATE_HOOK_BACKEND
  public:
#endif
    virtual void *create(void *target, void *replace) = 0;

    virtual void create(prepared_hook_data const &data)
    {
        *data.original = create(data.target, data.replace);
    }

    virtual void enable()  = 0;
    virtual void disable() = 0;

    virtual void enable(void *target)  = 0;
    virtual void disable(void *target) = 0;
};

} // namespace fd