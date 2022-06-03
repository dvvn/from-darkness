module;

#include <string_view>

export module cheat.hook_base;

export namespace cheat
{
    struct hook_base
    {
        virtual ~hook_base() = default;

        virtual bool enable() = 0;
        virtual bool disable() = 0;

        virtual bool is_static() const = 0;
        virtual std::string_view name() const = 0;

        virtual bool initialized() const = 0;
        virtual bool active() const = 0;

        virtual void init() = 0;
    };
} // namespace cheat
