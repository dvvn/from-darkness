#pragma once

#include <fd/gui/basic_menu.h>
#include <fd/gui/renderable.h>

#include <string>

namespace fd
{
enum class menu_state : uint8_t
{
    closed,
    hidden,
    shown
};

class menu_base : public basic_menu, public renderable_inner
{
    menu_state state_;
    bool wish_state_;

  public:
    menu_base();

    bool visible() const final;
    bool collapsed() const final;

    void show() final;
    void close() final;
    void toggle() final;

  protected:
    bool begin_frame() override;
    void end_frame() override;
};
} // namespace fd