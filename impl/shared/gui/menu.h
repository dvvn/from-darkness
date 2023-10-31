#pragma once
#include "noncopyable.h"
#include "gui/menu/item_id.h"

#include <imgui.h>

namespace fd::gui
{
template <class Callback, class UnloadHandler>
class menu final : public noncopyable
{
    bool visible_;
    bool next_visible_;

    [[no_unique_address]] //
    Callback callback_;
    [[no_unique_address]] //
    UnloadHandler unload_;

    void render_unload_button()
    {
        /*if (!ImGui::BeginChild(menu_item_id(__LINE__)))
            return;*/

        if (ImGui::Button("Unload"))
            std::invoke(unload_);

        // ImGui::EndChild();
    }

  public:
    menu(Callback callback, UnloadHandler unload)
        : visible_(false)
        , next_visible_(true)
        , callback_(std::move(callback))
        , unload_(std::move(unload))
    {
    }

    void toggle()
    {
        next_visible_ = !next_visible_;
    }

    void new_frame()
    {
        visible_ = next_visible_;
    }

    bool visible() const
    {
        return visible_;
    }

    void render()
    {
        assert(visible_);
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        ImGui::ShowDemoWindow();
#endif
        if (ImGui::Begin("WIP", &next_visible_, ImGuiWindowFlags_AlwaysAutoResize))
        {
            std::invoke(callback_);
            if constexpr (std::invocable<UnloadHandler>)
                render_unload_button();
        }
        ImGui::End();
    }

    // void render(basic_menu_item* item)
    //{
    //     item->render();
    // }
    //
    // void render(menu_item_getter const* items)
    //{
    //     ImGui::PushID(items);
    //     apply(*items);
    //     ImGui::PopID();
    // }
};
} // namespace fd::gui