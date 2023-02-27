#pragma once

#define FD_GUI_RANDOM_TAB_BAR_NAME

#include <fd/gui/basic_menu.h>

#include <string>

namespace fd
{
class tab_base
{
    std::string_view name_;

  public:
    virtual ~tab_base() = default;
    tab_base(std::string_view name);

  protected:
    bool new_frame();
    void end_frame();
};

class tab_bar_base
{
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
    std::string
#else
    std::string_view
#endif
        name_;

  public:
    tab_bar_base(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
        std::string_view name
#endif
    );

  protected:
    bool new_frame();
    void end_frame();
};

class menu_base : public basic_menu
{
    bool visible_;
    bool visibleNext_;

  public:
    menu_base();

    bool visible() const final;

    void show() final;
    void hide() final;
    void toggle() final;

  protected:
    bool new_frame(bool& visible);
    void end_frame(bool visible);
};
} // namespace fd