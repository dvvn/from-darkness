#pragma once

#include "gui/obj_holder.h"
#include "string/view.h"

#include <imgui_internal.h>

namespace ImGui::inline ex
{
// ReSharper disable once CppInconsistentNaming
inline bool BeginTabBar(ImGuiID const id, ImGuiTabBarFlags const flags = ImGuiTabBarFlags_None)
{
    auto const window  = GImGui->CurrentWindow;
    /*if (window->SkipItems)
        return false;*/
    auto const tab_bar = GImGui->TabBars.GetOrAddByKey(id);
    ImRect const tab_bar_bb{
        window->DC.CursorPos.x, window->DC.CursorPos.y, //
        window->WorkRect.Max.x, window->DC.CursorPos.y + GImGui->FontSize + GImGui->Style.FramePadding.y * 2};
    tab_bar->ID = id;
    return BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused);
}
} // namespace ImGui::inline ex

namespace fd::gui
{
template <typename Fn>
class tab_bar_item
{
    string_view name_;
    Fn callback_;

  public:
    tab_bar_item(string_view const name, Fn callback)
        : name_{name}
        , callback_{std::move(callback)}
    {
    }

    void operator()() const
    {
        if (!ImGui::BeginTabItem(name_))
            return;
        callback_();
        ImGui::EndTabItem();
    }
};

template <typename... Fn>
struct tab_bar
{
    using items_storage = detail::obj_holder<tab_bar_item<Fn>...>;

#ifdef _DEBUG
    using id_stored = string_view;
#else
    using id_stored = char const*;
#endif

  private:
    id_stored id_;
    items_storage items_;

  public:
    template <class... Item>
    tab_bar(id_stored const id, Item&&... item)
        : id_{id}
        , items_{std::forward<Item>(item)...}
    {
    }

    void operator()() const
    {
        bool begin;

#ifdef _DEBUG
        begin = ImGui::BeginTabBar(id_);
#else
        begin = ImGui::BeginTabBar(ImGui::GetID(static_cast<void const*>(id_)));
#endif

        if (!begin)
            return;
        items_();
        ImGui::EndTabBar();
    }
};

// template <typename... Fn>
// constexpr auto make_tab_bar(tab_bar<>::id_stored id, tab_bar_item<Fn>... item) -> tab_bar<Fn...>
// {
//     return {id, std::move(item)...};
// }

template <typename... Fn>
tab_bar(tab_bar<>::id_stored, tab_bar_item<Fn>...) -> tab_bar<Fn...>;
} // namespace fd::gui