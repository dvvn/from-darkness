module;

#include <fd/assert.h>

#include <imgui_internal.h>

module fd.gui.menu.impl;

using namespace fd;
using namespace gui;

static auto _Im_str(const string_view strv)
{
#ifndef IMGUI_HAS_STRV
    const auto data = strv.data();
    FD_ASSERT(data[strv.size()] == '\0');
    return data;
#else

#endif
}

namespace ImGui
{
    static bool BeginTabItem(const string_view label)
    {
        return BeginTabItem(_Im_str(label), nullptr);
    }

} // namespace ImGui

tab::tab(const string_view name)
    : name_(name)
{
}

bool tab::render() const
{
    const auto ret = ImGui::BeginTabItem(name_);
    if (ret)
        ImGui::EndTabItem();
    return ret;
}

void tab::render_data() const
{
    for (auto& fn : data_)
        fn();
}

//-------

tab_bar::tab_bar(const string_view name)
    : name_(name)
{
}

void tab_bar::render() const
{
    auto& g                 = *GImGui;
    const auto window       = g.CurrentWindow;
    /*if (window->SkipItems)
        return;*/
    const auto id           = window->GetID(name_.data(), name_.data() + name_.size());
    const auto tab_bar      = g.TabBars.GetOrAddByKey(id);
    const ImRect tab_bar_bb = { window->DC.CursorPos.x, window->DC.CursorPos.y, window->WorkRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2 };
    tab_bar->ID             = id;
    constexpr auto flags    = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip;
    if (!ImGui::BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused))
        return;

    const tab* active_tab = nullptr;
    for (const auto t : data_)
    {
        if (t->render())
            active_tab = t;
    }

    FD_ASSERT(active_tab != nullptr);
    active_tab->render_data();

    ImGui::EndTabBar();
}

void tab_bar::store_callback(const tab& new_tab)
{
    data_.emplace_back(&new_tab);
}

//-------

menu_impl::menu_impl()
    : visible_(false)
{
}

bool menu_impl::visible() const
{
    return visible_;
}

void menu_impl::show()
{
    next_visible_ = true;
}

void menu_impl::hide()
{
    next_visible_ = false;
}

void menu_impl::toggle()
{
    next_visible_ = !visible_;
}

void menu_impl::render()
{
    if (!next_visible_)
    {
        visible_ = false;
        return;
    }

    auto visible = true;

    if (ImGui::Begin("Unnamed", &visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        for (const auto tb : data_)
            tb->render();
    }
    ImGui::End();

    next_visible_ = visible_ = visible;
}

void menu_impl::store_callback(const tab_bar& new_tab_bar)
{
    data_.emplace_back(&new_tab_bar);
}
