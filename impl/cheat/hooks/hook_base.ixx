module;

#include <string_view>

export module cheat.hooks.base;

export namespace cheat::hooks
{
    class base
    {
    public:
        virtual ~base( ) = default;

        virtual bool enable( ) = 0;
        virtual bool disable( ) = 0;

        virtual bool is_static( ) const noexcept = 0;
        virtual std::string_view name( ) const noexcept = 0;
    };

    class class_base : public virtual base
    {
    public:
        bool is_static( ) const noexcept final
        {
            return false;
        }
    };

    class static_base : public virtual base
    {
    public:
        bool is_static( ) const noexcept final
        {
            return true;
        }
    };
}