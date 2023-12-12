#pragma once
#include "container/vector/static.h"
#include "entity_cache/player.h"
#include "native/handle.h"
#include "native/player_controller.h"
#include "type_traits/conditional.h"
#include "noncopyable.h"

#include <algorithm>
#include <atomic>
#include <cassert>

namespace fd
{
template <class T, bool ThreadSafe>
class cached_entity;

template <class T>
class cached_entity<T, false>
{
    T ent_;
    bool removed_;

  public:
    cached_entity(native::entity_instance* ent)
        : ent_{ent}
        , removed_{true}
    {
    }

    cached_entity* get()
    {
        return this;
    }

    T const* operator->() const
    {
        return &ent_;
    }

    T* operator->()
    {
        return &ent_;
    }

    bool in_use() const
    {
        return removed_ == false;
    }

    bool removed() const
    {
        return removed_;
    }

    void mark_removed()
    {
        removed_ = true;
    }
};

template <class T>
class cached_entity<T, true>
{
    static constexpr auto memory_order = std::memory_order_relaxed;

    class entity_view
    {
        cached_entity* self_;

      public:
        ~entity_view()
        {
            if (!self_)
                return;
            --self_->use_count_;
        }

        entity_view(cached_entity* self)
            : self_{self}
        {
            ++self_->use_count_;
        }

        entity_view(entity_view const& other)
            : self_(other.self_)
        {
            ++self_->use_count_;
        }

        entity_view& operator=(entity_view const& other)
        {
            self_ = other.self_;
            ++self_->use_count_;
            return *this;
        }

        entity_view(entity_view&& other) noexcept
            : self_(std::exchange(other.self_, nullptr))
        {
        }

        entity_view& operator=(entity_view&& other) noexcept
        {
            using std::swap;
            swap(self_, other.self_);
            return *this;
        }

        T* operator->() const
        {
            return &self_->ent_;
        }
    };

    T ent_;
    std::atomic_size_t use_count_;
    std::atomic_bool removed_;

  public:
    cached_entity(native::entity_instance* ent)
        : ent_{ent}
        , use_count_{1}
        , removed_{false}
    {
    }

    entity_view get()
    {
        return {this};
    }

    T const* operator->() const
    {
        return &ent_;
    }

    T* operator->()
    {
        return &ent_;
    }

    size_t use_count() const
    {
        return use_count_.load(memory_order);
    }

    bool in_use() const
    {
        return use_count() != 0;
    }

    bool removed() const
    {
        return removed_.load(memory_order);
    }

    void mark_removed()
    {
        assert(in_use() && !removed());

        --use_count_;
        removed_.store(true, memory_order);
    }
};

class entity_cache : public noncopyable
{
    static constexpr bool store_removed_players = false; //  true to have ents removed from game
    static constexpr size_t max_players         = 64;
    static constexpr size_t max_players_stored  = store_removed_players ? max_players : static_cast<size_t>(max_players * 1.5);

    using cached_player = cached_entity<player, store_removed_players>;
    static_vector<cached_player, max_players_stored> players_;

    void store(native::cs_player_controller* const pl)
    {
#ifdef _DEBUG
        if (players_.empty())
        {
            players_.emplace_back(pl);
            return;
        }
#endif

        auto const first = players_.data();
        auto const last  = first + players_.size();

        auto const unused = std::find_if_not(first, last, [](cached_player const& c_pl) {
            return c_pl.in_use();
        });

        if (unused != last)
        {
            std::destroy_at(unused);
            std::construct_at(unused, pl);
        }
        else
        {
            players_.emplace_back(pl);
        }
    }

    void remove(native::cs_player_controller* const pl)
    {
        auto const first = players_.data();
        auto const last  = first + players_.size();

        auto const target = std::find_if(first, last, [pl](cached_player const& c_pl) {
            if constexpr (store_removed_players)
                if (c_pl.removed())
                    return false;
            return c_pl->equal(pl);
        });

        if (target != last)
        {
            target->mark_removed();
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
};
} // namespace fd