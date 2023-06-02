#include "range.h"

#include <boost/lambda2.hpp>

#include <algorithm>

namespace fd
{
auto player_range::begin() const -> iterator
{
    return storage_.data();
}

auto player_range::end() const -> iterator
{
    return storage_.data() + storage_.size();
}

void player_range::add(player *p)
{
    storage_.emplace_back(p);
}

bool player_range::remove(player const &p)
{
    using namespace boost::lambda2;

    auto ed = storage_.end();
    auto it = std::find_if(storage_.begin(), ed, _1->*&player::native == p.native());

    auto found = it != ed;
    if (found)
        storage_.erase(it);
    return found;
}

void player_range::clear()
{
    storage_.clear();
}
}