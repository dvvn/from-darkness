#pragma once

#include "gui/menu.h"
#include "gui/menu/tab.h"

namespace fd
{
template <typename UnloadHandler>
auto make_menu_example(UnloadHandler&& unload_handler)
{
    using namespace gui;

    return menu{
        [] {
            menu_tab("Tab1", [] {
                menu_tab_item("One", [] {
                    ImGui::TextUnformatted("Tab1 text");
                    ImGui::TextUnformatted("Tab1 text2");
                });
                menu_tab_item("Two", [] {
                    ImGui::TextUnformatted("Tab1 text3");
                    ImGui::TextUnformatted("Tab1 text4");
                });
            });
            menu_tab("Tab2", [] {
                menu_tab_item("One", [] {
                    ImGui::TextUnformatted("Tab2 text");
                    ImGui::TextUnformatted("Tab2 text2");
                });
                menu_tab_item("Two", [] {
                    ImGui::TextUnformatted("Tab2 text3");
                    ImGui::TextUnformatted("Tab2 text4");
                });
            });
        },
        std::forward<UnloadHandler>(unload_handler)};
}
} // namespace fd