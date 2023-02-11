#include <fd/format.h>
#include <fd/gui/menu_impl.h>
#include <fd/views.h>

#include <imgui_internal.h>

namespace fd::gui
{
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
} // namespace fd::gui

// ReSharper disable CppInconsistentNaming
namespace ImGui
{
static bool BeginTabItem(const fd::string_view label)
{
    return BeginTabItem(fd::gui::_im_str(label), nullptr);
}
} // namespace ImGui

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
tab_base::tab_base(const string_view name)
    : name_(name)
{
}

bool tab_base::new_frame()
{
    return ImGui::BeginTabItem(name_);
}

void tab_base::end_frame()
{
    ImGui::EndTabItem();
}

//-------

#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
static size_t _TabCounter = 0;

tab_bar_base::tab_bar_base()
{
    write_string(name_, "tab", format_int(_TabCounter++));
}
#else
tab_bar_base::tab_bar_base(const string_view name)
    : name_(name)
{
}
#endif

bool tab_bar_base::new_frame()
{
    auto&      g      = *GImGui;
    const auto window = g.CurrentWindow;
    /*if (window->SkipItems)
        return;*/

    const auto id     = window->GetID(_begin(name_), _end(name_));
    const auto tabBar = g.TabBars.GetOrAddByKey(id);
    tabBar->ID        = id;
    const ImRect   tabBarBB(window->DC.CursorPos.x, window->DC.CursorPos.y, window->WorkRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    constexpr auto defaultFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip;
    constexpr auto extraFlags   = ImGuiTabBarFlags_IsFocused;

    return (ImGui::BeginTabBarEx(tabBar, tabBarBB, defaultFlags | extraFlags));
}

void tab_bar_base::end_frame()
{
    ImGui::EndTabBar();
}

//-------

menu_impl_base::menu_impl_base()
    : visible_(false)
    , visibleNext_(true)
{
}

bool menu_impl_base::visible() const
{
    return visible_;
}

void menu_impl_base::show()
{
    visibleNext_ = true;
}

void menu_impl_base::hide()
{
    visibleNext_ = false;
}

void menu_impl_base::toggle()
{
    visibleNext_ = !visible_;
}

bool menu_impl_base::new_frame(bool& visible)
{
    if (!visibleNext_)
    {
        visible_ = false;
        return false;
    }

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

        // render tabs
    }

    return true;
}

void menu_impl_base::end_frame(bool visible)
{
    ImGui::End();
    visibleNext_ = visible_ = visible;
}
}