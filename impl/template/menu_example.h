#pragma once

#include "gui/menu.h"
#include "gui/menu/tab.h"

namespace fd
{
template <typename UnloadHandler>
auto make_menu_example(UnloadHandler unload_handler)
{
    using gui::detail::obj_holder;
    using namespace gui;

    constexpr auto make_item = [](std::string_view name, std::string_view str) {
        return tab_bar_item{
            name, //
            [str] {
                ImGui::TextUnformatted(str);
            }};
    };

    return menu{
        obj_holder{
            tab_bar{"Tab1", make_item("Tab1_1", "Text1"), make_item("Tab1_2", "Text2")},
            tab_bar{"Tab2", make_item("Tab2_1", "Text1")},
            tab_bar{"Tab3", make_item("Tab3_1", "Text1"), make_item("Tab3_2", "Text2"), make_item("Tab3_3", "Text3")}},
        std::move(unload_handler)
    };    
}
} // namespace fd