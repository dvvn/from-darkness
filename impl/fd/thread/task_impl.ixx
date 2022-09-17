module;

#include <utility>

export module fd.task.impl;
export import fd.task;
export import fd.semaphore;
export import fd.functional.invoke;

using namespace fd;

template <typename Fn>
class lockable_task : public task
{
    Fn fn_;
    semaphore sem_;

  public:
    template <typename Fn1>
    lockable_task(Fn1&& fn, const bool locked = true)
        : fn_(std::forward<Fn1>(fn))
        , sem_(locked ? 0 : 1, 1)

    {
    }

    void start() override
    {
        if constexpr (invocable<Fn>)
        {
            invoke(fn_);
            sem_.release();
        }
        else
        {
            invoke(fn_, sem_);
        }
    }

    void wait() override
    {
        sem_.acquire();
    }
};

template <typename Fn>
lockable_task(Fn&&, const bool = true) -> lockable_task<std::decay_t<Fn>>;

export namespace fd
{
    using ::lockable_task;
} // namespace fd
