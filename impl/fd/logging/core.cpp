#include "core.h"

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <functional>

namespace fd
{
using storage_base = boost::container::static_vector<core_logger *, 4>;

class storage
{
    storage_base storage_;
#ifdef _DEBUG
    bool init_      = false;
    bool destroyed_ = false;
#endif

  public:
    ~storage()
    {
        (void)this;
        assert(init_);
        assert(destroyed_);
    }

    bool contains(void *value) const
    {
        return std::any_of(storage_.begin(), storage_.end(), [=](void *other) { return value == other; });
    }

    void do_init()
    {
        assert(!init_);
        std::for_each(storage_.begin(), storage_.end(), std::mem_fn(&core_logger::init));
        storage_.shrink_to_fit();
#ifdef _DEBUG
        init_ = true;
#endif
    }

    void do_destroy()
    {
        assert(!destroyed_);
        assert(init_);
        std::for_each(storage_.begin(), storage_.end(), std::mem_fn(&core_logger::destroy));
#ifdef _DEBUG
        destroyed_ = true;
#endif
    }

    void add(core_logger *logger)
    {
        assert(!init_);
        assert(!contains(logger));
        storage_.emplace_back(logger);
    }
};

static std::add_lvalue_reference_t<storage> s()
{
    static storage s;
    return s;
}

void init_logging()
{
    s().do_init();
}

void stop_logging()
{
    s().do_destroy();
}

core_logger::core_logger()
{
    s().add(this);
}
} // namespace fd