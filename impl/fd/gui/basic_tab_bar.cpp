#include <fd/gui/basic_tab_bar.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace fd
{
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
static uint16_t tab_counter = 0;

basic_tab_bar::basic_tab_bar()
{
    name_.append("##tbar").append(std::to_string(tab_counter++));
}
#else
tab_bar_base::tab_bar_base(std::string_view name)
    : name_(name)
{
}
#endif

bool basic_tab_bar::begin_frame()
{
    constexpr auto flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton |
                           ImGuiTabBarFlags_NoTooltip;
#ifdef IMGUI_HAS_IMSTR
    return ImGui::BeginTabBar(name_, flags);
#else
    auto &g     = *GImGui;
    auto window = g.CurrentWindow;
    if (window->SkipItems)
        return false;

    auto id         = window->GetID(name_.data(), name_.data() + name_.size());
    auto tab_bar    = g.TabBars.GetOrAddByKey(id);
    auto tab_bar_bb = ImRect(
        window->DC.CursorPos.x,
        window->DC.CursorPos.y,
        window->WorkRect.Max.x,
        window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    tab_bar->ID = id;
    return ImGui::BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused);
#endif
}

void basic_tab_bar::end_frame()
{
    (void)this;
    ImGui::EndTabBar();
}
} // namespace fd