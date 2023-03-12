#pragma once

namespace fd
{
struct basic_menu
{
    virtual ~basic_menu() = default;

    virtual bool visible() const   = 0;
    virtual bool collapsed() const = 0;

    virtual void show()  = 0;
    virtual void close() = 0;
    virtual void toggle();
};
} // namespace fd