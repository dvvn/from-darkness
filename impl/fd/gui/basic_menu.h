#pragma once

namespace fd
{
struct basic_menu
{
    virtual ~basic_menu();

    virtual bool visible() const = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void toggle();

    virtual bool render() = 0;
};
} // namespace fd