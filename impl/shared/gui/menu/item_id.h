#pragma once


#include <imgui.h>

#include <concepts>

// ReSharper disable CppInconsistentNaming

namespace ImGui::inline ex
{
ImGuiID GetID(int n);
#if defined(IMGUI_DISABLE_OBSOLETE_FUNCTIONS) && defined(IMGUI_HAS_IMSTR)
ImGuiID GetID(char const* first, char const* last);
#endif
} // namespace ImGui::inline ex

// ReSharper restore CppInconsistentNaming

namespace fd::gui
{
class menu_item_id
{
    ImGuiID id_;

  public:
    constexpr menu_item_id(ImGuiID const id)
        : id_(id)
    {
    }

    template <typename... Args>
    menu_item_id(Args&&... args) requires requires { ImGui::GetID(args...); }
        : id_(ImGui::GetID(args...))
    {
    }

    template <std::same_as<ImGuiID> T>
    operator T() const
    {
        return id_;
    }
};
}