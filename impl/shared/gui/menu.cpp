#include "menu.h"
#include "functional/basic_function.h"
#include "string/view.h"
#include "vars/basic_group.h"

#include <imgui.h>
#include <imgui_internal.h>

// ReSharper disable once CppInconsistentNaming
namespace ImGui
{
// ReSharper disable once CppInconsistentNaming
static bool BeginTabBar(std::type_identity_t<ImGuiID> id, ImGuiTabBarFlags flags = 0)
{
    auto &g     = *GImGui;
    auto window = g.CurrentWindow;
    /*if (window->SkipItems)
        return false;*/

    auto tab_bar = g.TabBars.GetOrAddByKey(id);
    ImRect tab_bar_bb(
        window->DC.CursorPos.x, window->DC.CursorPos.y, //
        window->WorkRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    tab_bar->ID = id;
    return BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused);
}

// ReSharper disable once CppInconsistentNaming
static ImGuiID GetID(std::type_identity_t<int> n)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    return window->GetID(n);
}
} // namespace ImGui

namespace fd
{
class menu final : public basic_menu
{
    bool visible_;
    bool next_visible_;

    unload_handler const*unload_;

  public:
    menu(unload_handler const*unload)
        : visible_(false)
        , next_visible_(true)
        , unload_(unload)
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
        return ImGui::Begin("WIP", &next_visible_, ImGuiWindowFlags_AlwaysAutoResize);
    }

    static void render_current(basic_variables_group *group)
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
        if (group && ImGui::BeginTabBar(ImGui::GetID(group)))
        {
            render_current(group);
            ImGui::EndTabBar();
        }
    }

    void render(basic_variables_group *group) override
    {
        ImGui::PushID(this);
        if (ImGui::BeginTabBar(ImGui::GetID(__LINE__)))
        {
            render_current(group);
            ImGui::EndTabBar();
        }
        if (ImGui::BeginChild(ImGui::GetID(__LINE__)))
        {
            if (ImGui::Button("Unload"))
                std::invoke(*unload_);
        }
        ImGui::EndChild();
        ImGui::PopID();
    }

    void end_scene() override
    {
        ImGui::End();
    }
};

FD_OBJECT_IMPL(menu);
}