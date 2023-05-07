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
    iterator begin() const override
    {
        return storage_.data();
    }

    iterator end() const override
    {
        return storage_.data() + storage_.size();
    }

    void add(player *p)
    {
        storage_.emplace_back(p);
    }

    bool remove(player const &p)
    {
        auto end = storage_.end();
        for (auto it = storage_.begin(); it != end; ++it)
        {
            if (**it == p)
            {
                storage_.erase(it);
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        storage_.clear();
    }
};

struct players_range_impl
{
};
}