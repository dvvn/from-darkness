module;

#include <fd/assert.h>
#include <fd/object.h>

#include <algorithm>
#include <array>
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

#include <veque.hpp>

module fd.async;

struct thread_pool_impl final : basic_thread_pool
{
    using thread_type = std::jthread;
    using mutex_type  = std::mutex;

  private:
    std::vector<thread_type> storage_;
    veque::veque<std::variant<function_type, function_type_ex>> tasks_;
    mutex_type mtx_;

    void worker(const std::stop_token& stop)
    {
        while (!tasks_.empty() && !stop.stop_requested())
        {
            mtx_.lock();
            auto task = std::move(tasks_.back());
            tasks_.pop_back();
            mtx_.unlock();

            switch (task.index())
            {
            case 0:
                fd::invoke(std::get<0>(task));
                break;
            case 1:
                fd::invoke(std::get<1>(task), stop);
                break;
            default:
                throw;
            };
        }
    }

    void spawn_thread()
    {
        const auto end      = storage_.end();
        const auto finished = std::find_if(storage_.begin(), end, [](const thread_type& t) {
            return !t.joinable();
        });
        if (finished == end)
            return;
        const auto thr = std::addressof(*finished);
        if constexpr (!std::is_trivially_destructible_v<thread_type>)
            std::destroy_at(thr);
        std::construct_at(thr, [this](const std::stop_token stop) {
            this->worker(stop);
        });
    }

    template <class Fn>
    void store_task(Fn& func)
    {
        const std::scoped_lock lock(mtx_);
        tasks_.emplace_back(std::move(func));
        spawn_thread();
    }

  public:
    thread_pool_impl()
    {
        const auto limit = thread_type::hardware_concurrency();
        FD_ASSERT(limit > 0);
        storage_.resize(limit);
    }

    void wait() override
    {
        for (auto& t : storage_)
        {
            if (t.joinable())
                t.join();
        }
    }

    task operator()(function_type func) override
    {
        store_task(func);
        return {};
    }

    task operator()(function_type_ex func) override
    {
        store_task(func);
        return {};
    }

    task operator()(function_type func, const lazy_tag_t) override
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }

    task operator()(function_type_ex func, const lazy_tag_t) override
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }
};

FD_OBJECT_BIND_TYPE(async, thread_pool_impl);
