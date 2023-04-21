#include <fd/logging/init.h>

#include <boost/container/static_vector.hpp>

#include <algorithm>

using std::ranges::contains;
using std::ranges::for_each;

namespace fd
{
using storage_base = boost::container::static_vector<core_logger *, 4>;

class storage_type
{
    storage_base storage_;

#ifdef _DEBUG
    bool init_      = false;
    bool destroyed_ = false;
#endif

  public:
    ~storage_type()
    {
        assert(init_);
        assert(destroyed_);
        (void)this;
    }

    void add(core_logger *logger)
    {
        assert(!init_);
        assert(!contains(storage_, logger));
        storage_.emplace_back(logger);
    }

    void init()
    {
        assert(!init_);
        for_each(storage_, &core_logger::init);
        storage_.shrink_to_fit();
#ifdef _DEBUG
        init_ = true;
#endif
    }

    void destroy()
    {
        assert(!destroyed_);
        assert(init_);
        for_each(storage_, &core_logger::destroy);
#ifdef _DEBUG
        destroyed_ = true;
#endif
    }
};

static storage_type &storage()
{
    static storage_type s;
    return s;
}

void init_logging()
{
    storage().init();
}

logger_registrar::logger_registrar()
{
    storage().add(this);
}

void logger_registrar::start()
{
    storage().init();
}

void logger_registrar::stop()
{
    storage().destroy();
}
}