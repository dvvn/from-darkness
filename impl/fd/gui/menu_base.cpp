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
    , wish_state_(wish_menu_state::shown)
{
}

bool menu_base::visible() const
{
    switch (state_)
    {
    case menu_state::closed:
        return false;
    case menu_state::shown:
    case menu_state::hidden:
        return true;
    default:
        std::unreachable();
    }
}

bool menu_base::collapsed() const
{
    return state_ == menu_state::hidden;
}

void menu_base::show()
{
    wish_state_ = wish_menu_state::shown;
}

void menu_base::close()
{
    wish_state_ = wish_menu_state::closed;
}

void menu_base::toggle()
{
    if (wish_state_ != wish_menu_state::unchanged)
        return;

    switch (state_)
    {
    case menu_state::closed:
        wish_state_ = wish_menu_state::shown;
        break;
    case menu_state::shown:
    case menu_state::hidden:
        wish_state_ = wish_menu_state::closed;
        break;
    default:
        std::unreachable();
    }
}

bool menu_base::begin_frame()
{
    switch (wish_state_)
    {
    case wish_menu_state::closed: {
        wish_state_ = wish_menu_state::unchanged;
        state_      = menu_state::closed;
        return false;
    }
    case wish_menu_state::shown: {
        wish_state_ = wish_menu_state::unchanged;
        break;
    }
    case wish_menu_state::unchanged: {
        if (state_ == menu_state::closed)
            return false;
        break;
    }
    default:
        std::unreachable();
    }

    constexpr auto flags = ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoSavedSettings /*| ImGuiWindowFlags_MenuBar*/;

    auto shown     = true;
    auto collapsed = !ImGui::Begin("Unnamed", &shown, flags);

    if (!shown)
    {
        state_ = menu_state::closed;
        end_frame();
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