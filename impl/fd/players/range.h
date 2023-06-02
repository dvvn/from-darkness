#pragma once

#include "basic_range.h"
#include "index.h"

#include <boost/container/static_vector.hpp>

namespace fd
{
class player_range final : public basic_player_range
{
    using storage_type = boost::container::static_vector<player *, max_player_count>;

    storage_type storage_;

  public:
    iterator begin() const override;
    iterator end() const override;

    void add(player *p);
    bool remove(player const &p);
    void clear();
};

struct players_range_impl
{
};
}