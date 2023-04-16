#include <fd/logging/default.h>
#include <fd/logging/init.h>

#include <boost/container/static_vector.hpp>

#include <algorithm>

namespace fd
{
static boost::container::static_vector<abstract_logger *, 4> storage;

void add_logger(abstract_logger &logger)
{
    assert(!std::ranges::contains(storage, &logger));
    storage.emplace_back(&logger);
}

void init_logging()
{
    for (auto *logger : storage)
    {
        logger->init();
    }

    // fill storage to prevent adding
    storage.resize(storage.capacity());
}
}