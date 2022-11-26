#pragma once

#include <fd/functional.h>

#include <memory>

namespace fd
{
    struct task
    {
        virtual ~task()      = default;
        virtual void start() = 0;
        virtual void wait()  = 0;
    };

    struct basic_thread_pool
    {
        using function_type = function<void() const>;
        using task_type     = std::shared_ptr<task>;

        virtual ~basic_thread_pool() = default;

        virtual void wait() = 0;

        virtual bool add_simple(function_type func)    = 0;
        virtual task_type add(function_type func)      = 0;
        virtual task_type add_lazy(function_type func) = 0;
    };

   extern basic_thread_pool* thread_pool;
} // namespace fd
