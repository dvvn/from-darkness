#include "widgets.h"

#include <imgui_internal.h>

namespace ImGui::ex
{
bool BeginTabBar(std::type_identity_t<ImGuiID> const id, ImGuiTabBarFlags const flags)
{
    auto const window = GImGui->CurrentWindow;
    /*if (window->SkipItems)
        return false;*/

    auto const tab_bar = GImGui->TabBars.GetOrAddByKey(id);
    ImRect const tab_bar_bb(
        window->DC.CursorPos.x, window->DC.CursorPos.y, //
        window->WorkRect.Max.x, window->DC.CursorPos.y + GImGui->FontSize + GImGui->Style.FramePadding.y * 2);
    tab_bar->ID = id;
     return BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused);
}
}