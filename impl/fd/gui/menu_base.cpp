#include <fd/gui/menu_base.h>

#include <imgui_internal.h>

#ifndef IMGUI_HAS_STRV
static std::string _ImTmpstring;
#endif
static auto _im_str(const std::string_view strv)
{
#ifndef IMGUI_HAS_STRV
    auto       data = strv.data();
    auto const size = strv.size();
    if (data[size] != '\0')
        data = _ImTmpstring.assign(data, size).data();
    return data;
#else

#endif
}

// ReSharper disable CppInconsistentNaming
namespace ImGui
{
static bool BeginTabItem(const std::string_view label)
{
    return BeginTabItem(_im_str(label), nullptr);
}
} // namespace ImGui

// ReSharper restore CppInconsistentNaming

namespace fd
{
tab_base::tab_base(const std::string_view name)
    : name_(name)
{
}

bool tab_base::new_frame()
{
    return ImGui::BeginTabItem(name_);
}

void tab_base::end_frame()
{
    (void)this;
    ImGui::EndTabItem();
}

//-------

#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
static uint16_t _TabCounter = 0;

tab_bar_base::tab_bar_base()
{
    fmt::format_to(std::back_inserter(name_), "{}{}", namePrefix_, _TabCounter++);
    name_.push_back('\0');
}
#else
tab_bar_base::tab_bar_base(const std::string_view name)
    : name_(name)
{
}
#endif

bool tab_bar_base::new_frame()
{
    auto&      g      = *GImGui;
    auto const window = g.CurrentWindow;
    /*if (window->SkipItems)
        return;*/

    auto const id     = window->GetID((name_.data()), name_.data() + name_.size());
    auto const tabBar = g.TabBars.GetOrAddByKey(id);
    tabBar->ID        = id;

    constexpr auto defaultFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton |
                                  ImGuiTabBarFlags_NoTooltip;
    constexpr auto extraFlags = ImGuiTabBarFlags_IsFocused;
    auto const     tabBarBB   = ImRect(
        window->DC.CursorPos.x,
        window->DC.CursorPos.y,
        window->WorkRect.Max.x,
        window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    return (ImGui::BeginTabBarEx(tabBar, tabBarBB, defaultFlags | extraFlags));
}

void tab_bar_base::end_frame()
{
    (void)this;
    ImGui::EndTabBar();
}

//-------

menu_base::menu_base()
    : visible_(false)
    , visibleNext_(true)
{
}

bool menu_base::visible() const
{
    return visible_;
}

void menu_base::show()
{
    visibleNext_ = true;
}

void menu_base::hide()
{
    visibleNext_ = false;
}

void menu_base::toggle()
{
    visibleNext_ = !visible_;
}

bool menu_base::new_frame(bool& visible)
{
    if (!visibleNext_)
    {
        visible_ = false;
        return false;
    }

    if (ImGui::Begin(
            "Unnamed",
            &visible,
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
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

void menu_base::end_frame(bool visible)
{
    ImGui::End();
    visibleNext_ = visible_ = visible;
}
}