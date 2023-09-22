#include "basic_menu_item.h"
#include "menu.h"
#include "object_holder.h"
#include "functional/basic_function.h"
#include "string/view.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>

// ReSharper disable once CppInconsistentNaming
namespace ImGui
{
// ReSharper disable once CppInconsistentNaming
static bool BeginTabBar(std::type_identity_t<ImGuiID> const id, ImGuiTabBarFlags const flags = 0)
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

// ReSharper disable once CppInconsistentNaming
static ImGuiID GetID(std::type_identity_t<int> const n)
{
    return GImGui->CurrentWindow->GetID(n);
}
} // namespace ImGui

namespace fd
{
class menu final : public basic_menu
{
    bool visible_;
    bool next_visible_;

    unload_handler const* unload_;

  public:
    menu(unload_handler const* unload)
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

    void render(basic_menu_item* item) override
    {
        // ImGui::PushID(item);
        if (ImGui::BeginTabBar(ImGui::GetID(__LINE__)))
        {
            render_item(item);
            ImGui::EndTabBar();
        }
        // ImGui::PopID();
    }

    void render(menu_item_getter const* items) override
    {
        ImGui::PushID(items);
        if (ImGui::BeginTabBar(ImGui::GetID(__LINE__)))
        {
            for (size_t item_num = 0;;)
            {
                auto const item = std::invoke(*items, item_num);
                if (!item)
                    break;
                render_item(item);
            }
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    void end_scene() override
    {
        render_internal();
        ImGui::End();
    }

  private:
    void render_item(basic_menu_item* item)
    {
        if (!ImGui::BeginTabItem(item->name()))
            return;
        item->render();
        if (auto const child = item->child())
            render_child(child);
        ImGui::EndTabItem();
    }

    void render_child(basic_menu_item* item)
    {
        ImGui::PushID(item);
        render(item);
        ImGui::PopID();
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

basic_menu* make_incomplete_object<menu>::operator()(unload_handler const* handler) const
{
    return make_object<menu>(handler);
}
}