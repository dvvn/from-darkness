#include <fd/logging/default.h>
#include <fd/logging/init.h>

#include <boost/container/static_vector.hpp>

namespace fd
{
using storage_base = boost::container::static_vector<abstract_logger *, 4>;

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

    bool contains(abstract_logger *l) const
    {
        for (auto *stored : storage_)
            return stored == l;
        return false;
    }

  public:
    void add(abstract_logger *logger)
    {
        assert(!init_);
        assert(!contains(logger));
        storage_.emplace_back(logger);
    }

    void init()
    {
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

void add_logger(abstract_logger *logger)
{
    storage().add(logger);
}

void init_logging()
{
    storage().init();
}
}