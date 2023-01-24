#include <fd/assert.h>
#include <fd/gui/menu_impl.h>

#include <imgui_internal.h>

#include <algorithm>
#include <span>

using namespace fd;
using namespace gui;

#ifndef IMGUI_HAS_STRV
static string _ImTmpString;
#endif

static auto _im_str(const string_view strv)
{
#ifndef IMGUI_HAS_STRV
    auto       data = strv.data();
    const auto size = strv.size();
    if (data[size] != '\0')
        data = _ImTmpString.assign(data, size).data();
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
    for (auto& cb : callbacks_)
        cb();
}

void tab::store(callback_type&& callback)
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
    auto&      g      = *GImGui;
    const auto window = g.CurrentWindow;
    /*if (window->SkipItems)
        return;*/
    const auto id     = window->GetID(name_.data(), name_.data() + name_.size());
    const auto tabBar = g.TabBars.GetOrAddByKey(id);
    tabBar->ID        = id;
    const ImRect   tabBarBB(window->DC.CursorPos.x, window->DC.CursorPos.y, window->WorkRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    constexpr auto defaultFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip;
    constexpr auto extraFlags   = ImGuiTabBarFlags_IsFocused;

    if (ImGui::BeginTabBarEx(tabBar, tabBarBB, defaultFlags | extraFlags))
    {
        // no end check here, active tab MUST be exsist
        for (auto itr = tabs_.begin(); /*itr != lastItr*/; ++itr)
        {
            const auto activeTab = *itr;
            // find active tab
            if (!activeTab->render())
                continue;
            // render all other tabs
            for (const auto t2 : std::span(itr + 1, tabs_.end()))
            {
                if (t2->render())
                    FD_ASSERT("Active tab already found!");
            }
            activeTab->render_data();
            break;
        }

        ImGui::EndTabBar();
    }
}

void tab_bar::store(tab& newTab)
{
    tabs_.emplace_back(&newTab);
}

//-------

menu::menu()
    : visible_(false)
    , visibleNext_(true)
{
}

bool menu::visible() const
{
    return visible_;
}

void menu::show()
{
    visibleNext_ = true;
}

void menu::hide()
{
    visibleNext_ = false;
}

void menu::toggle()
{
    visibleNext_ = !visible_;
}

bool menu::render()
{
    if (!visibleNext_)
    {
        visible_ = false;
        return false;
    }

    auto visible = true;

    if (ImGui::Begin("Unnamed", &visible, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::TextUnformatted("test");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        std::ranges::for_each(tabBars_, &tab_bar::render);
    }
    ImGui::End();

    return visibleNext_ = visible_ = visible;
}

void menu::store(tab_bar& newTabBar)
{
    tabBars_.emplace_back(&newTabBar);
}