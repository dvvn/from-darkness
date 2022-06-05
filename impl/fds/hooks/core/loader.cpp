module;

#include <fds/core/object.h>

#include <fds/core/assert.h>

#include <functional>
#include <future>
#include <span>
#include <thread>
#include <vector>

module fds.hooks_loader;

#if 0
template <typename T>
class simple_span
{
    T* ptr_;
    size_t size_;

  public:
    simple_span() = default;

    simple_span(T* ptr, size_t size)
        : ptr_(ptr)
        , size_(size)
    {
    }

    size_t size() const
    {
        return size_;
    }

    T& operator[](size_t idx) const
    {
        return ptr_[idx];
    }

    T* begin() const
    {
        return ptr_;
    }

    T* end() const
    {
        return ptr_ + size_;
    }
};
#endif

class thread : std::thread
{
  public:
    using std::thread::hardware_concurrency;

    template <typename... Ts>
    thread(Ts&&... args)
        : std::thread(std::forward<Ts>(args)...)
    {
    }

    void join()
    {
        if (std::thread::joinable())
            std::thread::join();
    }
};

struct starter_data
{
    std::atomic<uint8_t> pos;
    std::span<hook_base*> hooks;
    std::stop_source stop;
};

class hooks_loader final : public basic_hooks_loader
{
    thread main_thread_;
    std::vector<hook_base*> hooks_;

    static void main_thread_impl(starter_data* sdata)
    {
        for (;;)
        {
            if (sdata->stop.stop_requested())
                break;
            const auto current_pos = sdata->pos++;
            if (current_pos >= sdata->hooks.size())
                break;
            auto& hook = sdata->hooks[current_pos];
            if (!hook->initialized())
                hook->init();
            if (!hook->enable())
                sdata->stop.request_stop();
        }
    }

  public:
    std::future<bool> start() override;
    void stop() override;
    void add(hook_base* const hook) override;
};

FDS_OBJECT_BIND(basic_hooks_loader, loader, hooks_loader);

std::future<bool> hooks_loader::start()
{
    const auto threads_count = std::min(hooks_.size(), thread::hardware_concurrency());
    fds_assert(threads_count > 0, "Incorrect threads count");

    auto sdata   = std::make_shared<starter_data>();
    sdata->hooks = hooks_;
    auto prom    = std::make_shared<std::promise<bool>>();

    main_thread_ = [=] {
        std::vector<thread> threads;
        threads.reserve(threads_count);

        while (threads.size() != threads_count)
            threads.emplace_back(main_thread_impl, sdata.get());
        for (auto& t : threads)
            t.join();

        prom->set_value(!sdata->stop.stop_requested());
    };

    return prom->get_future();
}

void hooks_loader::stop()
{
    main_thread_.join();

    for (auto& h : hooks_)
    {
        if (!h->initialized())
            continue;

        h->disable();
    }
}

void hooks_loader::add(hook_base* const hook)
{
    hooks_.push_back(hook);
}
