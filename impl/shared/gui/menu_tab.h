#pragma once

#include "basic_menu_tab.h"
#include "menu_item_data.h"
#include "render/backend/imgui/helpers.h"
#include "render/backend/imgui/widgets.h"
#include "string/view.h"

namespace fd
{
template <class Items>
struct menu_tab : /*basic_menu_tab,*/ menu_item_data<Items>
{
    using menu_item_data<Items>::menu_item_data;

    string_view name() const
    {
        return menu_item_data<Items>::name;
    }

    void render() const
    {
        if (!ImGui::BeginTabBar(ImGui::GetID(this)))
            return;
        apply(menu_item_data<Items>::items, [](auto& item) {
            if (!ImGui::BeginTabItem(item.name()))
                return;
            item.render();
            ImGui::EndTabItem();
        });
        ImGui::EndTabBar();
    }
};

template <class Items>
menu_tab(string_view, Items) -> menu_tab<Items>;
}