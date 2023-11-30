#pragma once

#include "tier1/functional/bind.h"
#include "tier1/string/view.h"
#include "tier2/gui/menu.h"
#include "tier2/gui/menu/tab.h"

namespace FD_TIER(3)
{
template <typename UnloadHandler>
auto make_menu_example(UnloadHandler&& unload_handler)
{
    using namespace gui;

    return menu(
        [] {
            using namespace string_view_literals;
            menu_tab(
                "Tab1"sv,
                bind(menu_tab_item, "One"sv, bind_front(ImGui::TextUnformatted, "Text"sv)),
                bind(menu_tab_item, "Two"sv, bind_front(ImGui::TextUnformatted, "Text2"sv)));
            menu_tab(
                "Tab2"sv,
                bind(menu_tab_item, "__One"sv, bind_front(ImGui::TextUnformatted, "__Text"sv)),
                bind(menu_tab_item, "__Two"sv, bind_front(ImGui::TextUnformatted, "__Text2"sv)));
        },
        std::forward<UnloadHandler>(unload_handler));
}
}