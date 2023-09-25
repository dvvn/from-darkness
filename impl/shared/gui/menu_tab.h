#pragma once

#include "basic_menu_tab.h"
#include "menu_item_data.h"
#include "render/backend/imgui/helpers.h"
#include "render/backend/imgui/widgets.h"
#include "string/view.h"

namespace fd
{
template <size_t Count>
struct menu_tab : basic_menu_tab, menu_item_data<Count>
{
    using menu_item_data<Count>::menu_item_data;

    string_view name() const override
    {
        return menu_item_data<Count>::name;
    }

    void render() const override
    {
        if (!ImGui::BeginTabBar(ImGui::GetID(this)))
            return;
        apply(menu_item_data<Count>::items, [](basic_menu_item const* item) {
            if (!ImGui::BeginTabItem(item->name()))
                return;
            item->render();
            ImGui::EndTabItem();
        });
        ImGui::EndTabBar();
    }
};

template <size_t Count>
menu_tab(string_view, menu_items_packed<Count>) -> menu_tab<Count>;
}