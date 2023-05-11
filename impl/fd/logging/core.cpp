#include "core.h"

#include <fd/lazy_invoke.h>

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <functional>

namespace fd
{
using storage_base = boost::container::static_vector<core_logger *, 4>;

static class : public storage_base
{
    void *ready_ = this;

  public:
    bool ready() const
    {
        return ready_ == this;
    }

    bool contains(void *value) const
    {
        return std::any_of(begin(), end(), [=](void *other) { return value == other; });
    }

    void do_init()
    {
        std::for_each(begin(), end(), std::mem_fn(&core_logger::init));
    }

    void do_destroy()
    {
        std::for_each(begin(), end(), std::mem_fn(&core_logger::destroy));
    }

} storage_;

#ifdef _DEBUG
static bool init_      = false;
static bool destroyed_ = false;

[[maybe_unused]] //
static invoke_on_destruct validator = [] {
    assert(init_);
    assert(destroyed_);
};
#endif

static auto &pending()
{
    static storage_base s;
    return s;
}

static auto &have_pending()
{
    static auto v = false;
    return v;
}

static void add_logger_impl(core_logger *logger)
{
    assert(!storage_.contains(logger));
    storage_.emplace_back(logger);
}

static bool is_unique()
{
    switch (storage_.size())
    {
    case 0:
    case 1:
        std::unreachable();
    case 2:
        return storage_[0] != storage_[1];
    default:
        return std::all_of(storage_.rbegin(), storage_.rend(), [](void *p) {
            return *std::find(storage_.begin(), storage_.end(), p) == p;
        });
    }
}

static void add_pending()
{
    if (!std::exchange(have_pending(), false))
        return;

    auto &to_add = pending();

    using std::swap;
    swap(storage_, to_add);

    if (!to_add.empty())
    {
        assert(is_unique());
        std::for_each(to_add.begin(), to_add.end(), add_logger_impl);
    }
}

static void add_logger(core_logger *logger)
{
    if (!storage_.ready())
    {
        pending().emplace_back(logger);
        have_pending() = true;
    }
    else
    {
        assert(!init_);
        add_pending();
        add_logger_impl(logger);
    }
}

void init_logging()
{
    assert(storage_.ready());
    assert(!init_);
    add_pending();
    storage_.do_init();
    storage_.shrink_to_fit();
#ifdef _DEBUG
    init_ = true;
#endif
}

void stop_logging()
{
    assert(!destroyed_);
    assert(init_);
    storage_.do_destroy();
#ifdef _DEBUG
    destroyed_ = true;
#endif
}

core_logger::core_logger()
{
    add_logger(this);
}
} // namespace fd