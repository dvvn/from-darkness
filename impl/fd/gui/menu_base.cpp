#include <fd/gui/menu_base.h>

#include <imgui_internal.h>

#ifndef IMGUI_HAS_IMSTR
static std::string _ImTmpstring;

static auto _im_str(std::string_view strv)
{
    auto data = strv.data();
    auto size = strv.size();
    if (data[size] != '\0')
        data = _ImTmpstring.assign(data, size).data();
    return data;
}

namespace ImGui
{
static bool BeginTabItem(std::string_view label)
{
    return BeginTabItem(_im_str(label), nullptr);
}
} // namespace ImGui
#endif

namespace fd
{
menu_base::menu_base()
    : state_(menu_state::closed)
    , wish_state_(true)
{
}

bool menu_base::visible() const
{
    return state_ != menu_state::closed;
}

bool menu_base::collapsed() const
{
    return state_ == menu_state::hidden;
}

void menu_base::show()
{
    wish_state_ = true;
}

void menu_base::close()
{
    wish_state_ = false;
}

void menu_base::toggle()
{
    wish_state_ = state_ == menu_state::closed;
}

bool menu_base::begin_frame()
{
    if (!wish_state_)
    {
        state_ = menu_state::closed;
        return false;
    }

    constexpr auto flags = ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoSavedSettings /*| ImGuiWindowFlags_MenuBar*/;

    auto shown     = true;
    auto collapsed = !ImGui::Begin("Unnamed", &shown, flags);

    if (!shown)
    {
        state_ = menu_state::closed;
        return false;
    }

    if (collapsed)
        state_ = menu_state::hidden;
    else
        state_ = menu_state::shown;
    return true;
}

void menu_base::end_frame()
{
    ImGui::End();
}
} // namespace fd