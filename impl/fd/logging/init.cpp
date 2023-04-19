#include <fd/logging/init.h>

#include <boost/container/static_vector.hpp>

namespace fd
{
using storage_base = boost::container::static_vector<core_logger *, 4>;

class storage_type
{
    storage_base storage_;

#ifdef _DEBUG
    bool init_ = false;
#endif

    void do_init()
    {
        for (auto *l : storage_)
            l->init();
    }

    bool contains(core_logger *l) const
    {
        for (auto *stored : storage_)
            if (stored == l)
                return true;
        return false;
    }

  public:
    void add(core_logger *logger)
    {
        assert(!init_);
        assert(!contains(logger));
        storage_.emplace_back(logger);
    }

    void init()
    {
        assert(!init_);
        do_init();
#ifdef _DEBUG
        init_ = true;
#endif
        storage_.resize(0);
    }
};

static storage_type &storage()
{
    static storage_type s;
    return s;
}

void add_logger(core_logger *logger)
{
    storage().add(logger);
}

void init_logging()
{
    storage().init();
}
}