#include "basic_menu_item.h"
#include "menu.h"
#include "functional/basic_function.h"
#include "string/view.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>

// ReSharper disable once CppInconsistentNaming
namespace ImGui
{
// ReSharper disable once CppInconsistentNaming
static bool BeginTabBar(std::type_identity_t<ImGuiID> id, ImGuiTabBarFlags flags = 0)
{
    auto &g           = *GImGui;
    auto const window = g.CurrentWindow;
    /*if (window->SkipItems)
        return false;*/

    auto const tab_bar = g.TabBars.GetOrAddByKey(id);
    ImRect const tab_bar_bb(
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

    unload_handler const *unload_;

  public:
    menu(unload_handler const *unload)
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

    void render(basic_joined_menu_items const *items) override
    {
        render_group(items);
    }

    void end_scene() override
    {
        render_internal();
        ImGui::End();
    }

  private:
    void render_child(basic_menu_item *item)
    {
        ImGui::PushID(item);
        if (ImGui::BeginTabBar(ImGui::GetID(__LINE__)))
        {
            render(item);
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    void render_group(basic_joined_menu_items const *items)
    {
        ImGui::PushID(items);
        if (ImGui::BeginTabBar(ImGui::GetID(__LINE__)))
        {
            auto const end = items->end();
            for (auto it = items->begin(); it != end; ++it)
                render(*it);
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    void render(basic_menu_item *item)
    {
        if (!ImGui::BeginTabItem(item->name()))
            return;
        if (auto const child = item->child())
            render_child(child);
        if (auto const child = item->child_joined())
            render_group(child);

        item->render();
        ImGui::EndTabItem();
    }

    void render_internal()
    {
        if (ImGui::BeginChild(ImGui::GetID(__LINE__)))
        {
            if (ImGui::Button("Unload"))
                std::invoke(*unload_);
        }
        ImGui::EndChild();
    }
};

FD_OBJECT_IMPL(menu);
}