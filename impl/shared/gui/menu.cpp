#include "menu.h"
#include "string/view.h"
#include "vars/basic_group.h"

#include <imgui.h>
#include <imgui_internal.h>

// ReSharper disable once CppInconsistentNaming
namespace ImGui
{
// ReSharper disable once CppInconsistentNaming
static bool BeginTabBar(void const *pid, ImGuiTabBarFlags flags = 0)
{
    auto &g     = *GImGui;
    auto window = g.CurrentWindow;
    /*if (window->SkipItems)
        return false;*/

    auto id      = window->GetID(pid);
    auto tab_bar = g.TabBars.GetOrAddByKey(id);
    ImRect tab_bar_bb(
        window->DC.CursorPos.x,
        window->DC.CursorPos.y,
        window->WorkRect.Max.x,
        window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2
    );
    tab_bar->ID = id;
    return BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused);
}
} // namespace ImGui

namespace fd
{
class menu final : public basic_menu
{
    bool visible_;
    bool next_visible_;

  public:
    menu()
        : visible_(false)
        , next_visible_(true)
    {
    }

    void toggle() override
    {
        next_visible_ = !next_visible_;
    }

    void new_frame() override
    {
        visible_ = next_visible_;
    }

    bool visible() const override
    {
        return visible_;
    }

    bool begin_scene() override
    {
        assert(visible_);

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        ImGui::ShowDemoWindow();
#endif

        return ImGui::Begin("WIP", &next_visible_);
    }

    static void do_render(basic_variables_group *group)
    {
        for (; group != nullptr; group = group->next())
        {
            if (ImGui::BeginTabItem(group->name()))
            {
                group->on_gui();
                render_inner(group->inner());
                ImGui::EndTabItem();
            }
        }
    }

    static void render_inner(basic_variables_group *group)
    {
        if (group && ImGui::BeginTabBar(group))
        {
            do_render(group);
            ImGui::EndTabBar();
        }
    }

    void render(basic_variables_group *group) override
    {
        if (ImGui::BeginTabBar(this))
        {
            do_render(group);
            ImGui::EndTabBar();
        }
    }

    void end_scene() override
    {
        ImGui::End();
    }
};

FD_INTERFACE_IMPL(menu);
}