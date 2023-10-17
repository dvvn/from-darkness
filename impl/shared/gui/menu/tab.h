#pragma once

#include "gui/menu/item_id.h"
#include "string/view.h"

#include <functional>

namespace ImGui::inline ex
{
// ReSharper disable once CppInconsistentNaming
bool BeginTabBar(ImGuiID id, ImGuiTabBarFlags flags = ImGuiTabBarFlags_None);
} // namespace ImGui::inline ex

namespace fd
{
inline constexpr auto menu_tab_item = []<typename Fn>(string_view const name, Fn&& callback) -> void {
    if (!ImGui::BeginTabItem(name))
        return;

    (std::invoke(std::forward<Fn>(callback)));
    ImGui::EndTabItem();
};

inline constexpr auto menu_tab = []<typename... I>(menu_item_id const id, I&&... items) -> void {
    if (!ImGui::BeginTabBar(id))
        return;

    (std::invoke(std::forward<I>(items)), ...);
    ImGui::EndTabBar();
};
}