module;

#include <optional>

export module fd.mem_backup;

export namespace fd
{
    template <typename T>
    struct backup
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
        ~backup()
        {
            if (has_value())
            {
                _Restore();
            }
        }

        backup(const backup& other)            = delete;
        backup& operator=(const backup& other) = delete;

        backup(backup&& other)
        {
            *this = std::move(other);
        }

        backup& operator=(backup&& other)
        {
            using std::swap;
            swap(backup_, other.backup_);
            return *this;
        }

        backup() = default;

        backup(T& from)
        {
            backup_.emplace(from, &from);
        }

        /* template <typename T1>
        backup(T& from, T1&& owerride) //use std::exchange
            : backup(from)
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
