#include <fd/gui/basic_tab.h>

#include <imgui.h>

namespace fd
{
basic_tab::basic_tab(std::string_view name)
    : name_(name)
{
}

bool basic_tab::begin_frame()
{
    return ImGui::BeginTabItem(name_);
}

void basic_tab::end_frame()
{
    (void)this;
    ImGui::EndTabItem();
}
}