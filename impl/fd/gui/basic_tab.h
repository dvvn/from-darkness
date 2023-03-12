#pragma once

#include <fd/gui/renderable.h>

#include <string_view>

namespace fd
{
class basic_tab : public renderable_inner
{
    std::string_view name_;

  public:
    basic_tab(std::string_view name);

  protected:
    bool begin_frame() override;
    void end_frame() override;
};
} // namespace fd