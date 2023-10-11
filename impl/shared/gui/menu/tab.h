#pragma once

#include "render/backend/imgui/widgets.h"

namespace fd
{
template <class Items>
class menu_tab
{
    Items items_;

  public:
    constexpr menu_tab(Items items)
        : items_(std::move(items))
    {
    }

    void render() const
    {
        if (!ImGui::BeginTabBar(ImGui::GetID(this)))
            return;
        apply(items_);
        ImGui::EndTabBar();
    }
};

template <class Name, class Items>
class menu_tab_item
{
    Name name_;
    Items items_;

  public:
    constexpr menu_tab_item(Name name, Items items)
        : name_(std::move(name))
        , items_(std::move(items))
    {
    }

    void render() const
    {
        if (!ImGui::BeginTabItem(name_))
            return;
        apply(items_);
        ImGui::EndTabItem();
    }
};

template <class Name, class Items>
menu_tab_item(Name&&, Items) -> menu_tab_item<Name, Items>;

template <class Name, class Items>
void apply(menu_tab_item<Name, Items> const& item)
{
    render_menu_item(item);
}
}