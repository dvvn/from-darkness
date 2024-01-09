#include "container/vector/dynamic.h"
#include "noncopyable.h"
#include "test_holder.h"

#include <algorithm>

namespace fd
{
class test_holder : public noncopyable
{
    using test_fn = void (*)();

    union
    {
        uint8_t gap_;
        vector<test_fn> storage_;
    };

    bool in_use_;

  public:
    ~test_holder()
    {
        if (in_use_)
        {
            std::destroy_at(&storage_);
            // in_use_ = false;
        }
    }

    test_holder()
        : gap_{0}
        , in_use_{false}
    {
    }

    auto& operator+=(test_fn fn)
    {
        if (!in_use_)
        {
            std::construct_at(&storage_);
            in_use_ = true;
        }
        else
        {
            assert(!std::ranges::contains(storage_, fn));
        }
        storage_.emplace_back(fn);
        return *this;
    }

    void run()
    {
        if (!in_use_)
            return;
        std::for_each(storage_.data(), storage_.data() + storage_.size(), [](test_fn const fn) {
            fn();
        });
        std::destroy_at(&storage_);
        in_use_ = false;
    }
};

static test_holder& get_test_holder() noexcept
{
    static test_holder holder;
    return holder;
}

uint8_t add_test(void (*fn)()) noexcept
{
    get_test_holder() += fn;
    return 0;
}

void run_tests() noexcept
{
    get_test_holder().run();
}
} // namespace fd