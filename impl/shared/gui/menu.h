#pragma once
#include "noncopyable.h"
#include "render/backend/imgui/helpers.h"

#include <imgui.h>

namespace fd
{
template <class Items, class UnloadHandler>
class menu final : public noncopyable
{
    bool visible_;
    bool next_visible_;

    [[no_unique_address]] //
    Items items_;
    [[no_unique_address]] //
    UnloadHandler unload_;

  public:
    menu(Items items, UnloadHandler unload)
        : visible_(false)
        , next_visible_(true)
        , items_(std::move(items))
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
            apply(items_);

            if (ImGui::BeginChild(ImGui::GetID(__LINE__)))
            {
                if (ImGui::Button("Unload"))
                {
                    using std::invoke;
                    invoke(unload_);
                }
            }
            ImGui::EndChild();
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

} // namespace fd