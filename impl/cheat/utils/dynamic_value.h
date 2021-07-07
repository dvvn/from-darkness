#pragma once

#include "value_setter.h"

//DEPRECATED !!!

namespace cheat::utl
{
    template <typename T>
    class dynamic_value
    {
    public:
        dynamic_value( )
        {
            static_assert(std::is_default_constructible_v<T>);
            changed_ = false;
        }

        dynamic_value(const T& def_value)
        {
            Set_value(def_value);
        }

        dynamic_value(T&& def_value)
        {
            Set_value(std::move(def_value));
        }

        //PREVENT_COPY_FORCE_MOVE(dynamic_value);

    private:
        template <typename ...Args>
        void Set_value(Args&& ...args)
            requires(std::is_constructible_v<T, decltype(args)...>)
        {
            static_assert(std::is_default_constructible_v<T>);
            last_value_ = std::move(current_value_);
            current_value_ = T(std::forward<Args>(args)...);
        }

        class Lock_helper
        {
        public:
            Lock_helper(dynamic_value* owner) : owner_(owner)
            {
                ++this->owner_->locks_;
            }

            ~Lock_helper( )
            {
                --this->owner_->locks_;
            }

            Lock_helper(const Lock_helper& other) : owner_(other.owner_)
            {
                ++this->owner_->locks_;
            }

            Lock_helper& operator==(const Lock_helper&)= delete;

            bool operator()( ) const
            {
                return this->owner_->locks_ > 1;
            }

        private:
            dynamic_value* owner_;
        };
        class Set_helper
        {
        public:
            Set_helper(dynamic_value* owner) : owner_(owner)
            {
                ++owner_->setters_;
            }

            ~Set_helper( )
            {
                --owner_->setters_;
                if (owner_->setters_ > 0)
                    return;

                this->owner_->changed_ = called_;
            }

            Set_helper(const Set_helper& other) : owner_(other.owner_), called_(other.called_)
            {
                ++owner_->setters_;
            }

            Set_helper& operator==(const Set_helper&)= delete;

            void operator()(T&& temp_value)
            {
                this->owner_->Set_value(std::forward<T>(temp_value));
                called_ = true;
            }

        private:
            dynamic_value* owner_ = nullptr;
            bool called_ = false;
        };

    public:
        _NODISCARD auto new_value( )
        {
            return cheat::tools::make_value_setter(Lock_helper(this), [&]( )-> T& { return current_value_; }, Set_helper(this), current_value_);
        }

        template <typename ...Args>
        bool set_new_value(Args&& ...args)
            requires(std::constructible_from<T, decltype(args)...>)
        {
            Locked_assert( );

            T new_val = T(FWD(args)...);
            bool equal;

            if constexpr (std::equality_comparable<T>)
            {
                equal = current_value_ == new_val;
            }
            else
            {
                static_assert(std::is_trivially_destructible_v<T>, "Unable to view type as bytes!");
                equal = std::memcmp(std::addressof(new_val), std::addressof(current_value_), sizeof(T));
            }

            if (equal)
                changed_ = false;
            else
            {
                Set_value(std::move(new_val));
                changed_ = true;
            }
            return changed_;
        }

        const T& last( ) const { return this->last_value_; }
        const T& current( ) const { return this->current_value_; }

        bool changed( ) const { return changed_; }

        void make_unchanged(bool sync = true)
        {
            BOOST_ASSERT_MSG(changed_ == true, "Already unchanged");
            Locked_assert( );

            if (sync)
                last_value_ = current_value_;
            changed_ = false;
        }

    private:
        void Locked_assert( ) const
        {
            BOOST_ASSERT_MSG(locks_ == 0, "Updater is locked.");
        }

        T last_value_;
        T current_value_;

        bool changed_ = true;
        size_t locks_ = 0, setters_ = 0;
    };
}
