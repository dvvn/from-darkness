#pragma once

#include <optional>

namespace fd
{
    template <typename T>
    struct mem_backup
    {
        using value_type = T;
        using pointer    = T*;
        using reference  = const T&;

      private:
        struct value_stored
        {
            value_type value;
            pointer owner;
        };

        std::optional<value_stored> backup_;

        void _Restore()
        {
            auto& b = *backup_;
            using std::swap;
            swap(*b.owner, b.value);
        }

      public:
        ~mem_backup()
        {
            if (has_value())
            {
                _Restore();
            }
        }

        mem_backup(const mem_backup& other)            = delete;
        mem_backup& operator=(const mem_backup& other) = delete;

        mem_backup(mem_backup&& other)
        {
            *this = std::move(other);
        }

        mem_backup& operator=(mem_backup&& other)
        {
            using std::swap;
            swap(backup_, other.backup_);
            return *this;
        }

        mem_backup() = default;

        mem_backup(T& from)
        {
            backup_.emplace(from, &from);
        }

        /* template <typename T1>
        mem_backup(T& from, T1&& owerride) //use std::exchange
            : mem_backup(from)
        {
            from = T(std::forward<T1>(owerride));
        } */

        const T& get() const
        {
            return backup_->value;
        }

        void restore()
        {
            if (has_value())
            {
                _Restore();
                reset();
            }
        }

        void reset()
        {
            backup_.reset();
        }

        bool has_value() const
        {
            return backup_.has_value();
        }
    };
} // namespace fd
