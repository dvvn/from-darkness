#pragma once

#include <fd/gui/renderable.h>

#include <string>

#define FD_GUI_RANDOM_TAB_BAR_NAME

namespace fd
{
class basic_tab_bar : public renderable_inner
{
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
    using string_type = std::string;
#else
    using string_type = std::string_view;
#endif
    string_type name_;

  public:
    basic_tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
        string_type name
#endif
    );

  protected:
    bool begin_frame() override;
    void end_frame() override;
};
} // namespace fd