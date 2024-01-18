#pragma once
#include "entity_cache/player.h"
#include "native/handle.h"
#include "native/player_controller.h"

#include <boost/container/static_vector.hpp>
#include <boost/noncopyable.hpp>

#include <algorithm>
#include <cassert>
#include <memory>

namespace fd
{
class entity_cache : public boost::noncopyable
{
    using cached_player = std::shared_ptr<player>;
    using cached_xxx    = void; // reserved

    using players_storage = boost::container::static_vector<cached_player, 64>;
    using xxx_storage     = void; // reserved

    players_storage players_;

    static constexpr auto unused_object = []<class Sptr>(Sptr const& shared) {
        return shared.use_count() == 0;
    };

    static cached_player construct(native::cs_player_controller* const controller)
    {
        auto pl      = std::make_shared<player>();
        pl->instance = controller;
        return pl;
    }

    template <class Rng>
    static void trim(Rng& rng)
    {
#ifdef _DEBUG
        auto const first = rng.data();
        auto const last  = first + rng.size();
#else
        auto const first = rng.begin();
        auto const last  = rng.end();
#endif
        auto unused_first = std::remove_if(first, last, unused_object);

#ifdef _DEBUG
        rng.resize(std::distance(first, unused_first));
#else
        rng.erase(unused_first, last);
#endif
    }

    void store(native::cs_player_controller* const pl)
    {
        auto new_player = construct(pl);

        if (players_.size() != players_.capacity())
        {
            players_.push_back(std::move(new_player));
            return;
        }

#ifdef _DEBUG
        auto const first = players_.data();
        auto const last  = first + players_.size();
#else
        auto const first = players_.begin();
        auto const last  = players_.end();
#endif

        auto const unused = std::find_if(first, last, unused_object);
        assert(unused != last);
        unused->swap(new_player);
    }

    void remove(native::cs_player_controller* const pl)
    {
#ifdef _DEBUG
        auto const first = players_.data();
        auto const last  = first + players_.size();
#else
        auto const first = players_.begin();
        auto const last  = players_.end();
#endif

        auto const target = std::find_if(first, last, [pl](cached_player const& c_pl) {
            return c_pl->equal(pl);
        });

        if (target != last)
        {
            if (target == last - 1)
                players_.pop_back();
            else
                *target = nullptr;
        }
    }

  public:
    void store(native::entity_instance* const instance, native::CBaseHandle const handle)
    {
        // handle.entry_index();
    }

    void remove(native::entity_instance* const instance, native::CBaseHandle const handle)
    {
    }

    void trim()
    {
        trim(players_);
    }
};
} // namespace fd