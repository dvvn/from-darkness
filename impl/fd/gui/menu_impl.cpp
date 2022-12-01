#include <fd/assert.h>
#include <fd/gui/menu_impl.h>

#include <imgui_internal.h>

#include <algorithm>

using namespace fd;
using namespace gui;

static auto _im_str(const string_view strv)
{
#ifndef IMGUI_HAS_STRV
    const auto data = strv.data();
    FD_ASSERT(data[strv.size()] == '\0');
    return data;
#else

#endif
}

// ReSharper disable CppInconsistentNaming
namespace ImGui
{
    static bool BeginTabItem(const string_view label)
    {
        return BeginTabItem(_im_str(label), nullptr);
    }

} // namespace ImGui

// ReSharper restore CppInconsistentNaming

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
    (void)std::ranges::for_each(callbacks_, Invoker);
}

void tab::store(callback_type callback)
{
    callbacks_.emplace_back(std::move(callback));
}

//-------

tab_bar::tab_bar(const string_view name)
    : name_(name)
{
}

void tab_bar::render() const
{
    auto& g           = *GImGui;
    const auto window = g.CurrentWindow;
    /*if (window->SkipItems)
        return;*/
    const auto id     = window->GetID(name_.data(), name_.data() + name_.size());
    const auto tabBar = g.TabBars.GetOrAddByKey(id);
    const ImRect tabBarBB(window->DC.CursorPos.x, window->DC.CursorPos.y, window->WorkRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    tabBar->ID                  = id;
    constexpr auto defaultFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip;
    constexpr auto extraFlags   = ImGuiTabBarFlags_IsFocused;
    if (!ImGui::BeginTabBarEx(tabBar, tabBarBB, defaultFlags | extraFlags))
        return;

    const auto activeTab = std::ranges::find_if(tabs_, &tab::render);
    (void)std::ranges::for_each(activeTab + 1, tabs_.end(), &tab::render);
    (*activeTab)->render_data();

    ImGui::EndTabBar();
}

void tab_bar::store(tab& newTab)
{
    tabs_.emplace_back(&newTab);
}

//-------

menu_impl::menu_impl()
    : visible_(false)
    , next_visible_(true)
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

bool menu_impl::render()
{
    if (!next_visible_)
    {
        visible_ = false;
        return false;
    }

    auto visible = true;

    if (ImGui::Begin("Unnamed", &visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        std::ranges::for_each(tab_bars_, &tab_bar::render);
    }
    ImGui::End();

    return next_visible_ = visible_ = visible;
}

void menu_impl::store(tab_bar& newTabBar)
{
    tab_bars_.emplace_back(&newTabBar);
}
