module;

#include <cheat/core/object.h>

#include <nstd/runtime_assert.h>

#include <functional>
#include <future>
#include <thread>
#include <vector>

module cheat.hooks.loader;

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

    size_t size() const noexcept
    {
        return size_;
    }

    T& operator[](size_t idx) const noexcept
    {
        return ptr_[idx];
    }

    T* begin() const noexcept
    {
        return ptr_;
    }

    T* end() const noexcept
    {
        return ptr_ + size_;
    }
};

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
    simple_span<basic_instance_info> hooks;
    std::stop_source stop;
};

class hooks_loader final : public basic_hooks_loader
{
    thread main_thread_;
    std::vector<instance_info_ptr> hooks_;

    static void main_thread_impl(starter_data* sdata)
    {
        for (;;)
        {
            if (sdata->stop.stop_requested())
                break;
            const auto current_pos = sdata->pos++;
            if (current_pos >= sdata->hooks.size())
                break;
            if (!sdata->hooks[current_pos]->enable())
                sdata->stop.request_stop();
        }
    }

  public:
    std::future<bool> start() override;
    void stop() override;
    void add(instance_info_ptr&& info) override;
};

CHEAT_OBJECT_BIND(basic_hooks_loader, _Loader_idx, hooks_loader, _Loader_idx);

std::future<bool> hooks_loader::start()
{
    const auto threads_count = std::min(hooks_.size(), thread::hardware_concurrency());
    runtime_assert(threads_count > 0, "Incorrect threads count");

    auto sdata = std::make_shared<starter_data>();
    static_assert(sizeof(instance_info_ptr) == sizeof(basic_instance_info*));
    sdata->hooks = {(basic_instance_info*)hooks_.data(), hooks_.size()};
    auto prom = std::make_shared<std::promise<bool>>();

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

        h->get()->disable();
    }
}

void hooks_loader::add(instance_info_ptr&& info)
{
    hooks_.push_back(std::move(info));
}
