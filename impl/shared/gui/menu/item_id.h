#pragma once

#include <imgui.h>

// ReSharper disable CppInconsistentNaming

namespace ImGui::inline ex
{
ImGuiID GetID(int n);
ImGuiID GetID(char const* first, char const* last);
} // namespace ImGui::inline ex

// ReSharper restore CppInconsistentNaming

namespace fd
{
class menu_item_id
{
    ImGuiID id_;

  public:
    constexpr menu_item_id(ImGuiID const id, int)
        : id_(id)
    {
    }

    menu_item_id(char const* first, char const* last)
        : menu_item_id(ImGui::GetID(first, last), __LINE__)
    {
    }

#ifdef IMGUI_HAS_IMSTR
    template <size_t S>
    menu_item_id(char const (&str)[S]) // skip ImStrv's strlen
        : menu_item_id(str, str + S - 1)
    {
    }

    template <class T>
    menu_item_id(T const& str) requires(std::is_class_v<T> && std::constructible_from<ImStrv, T>)
        : menu_item_id(ImGui::GetID(str))
    {
    }

    template <typename T>
    menu_item_id(T* ptr) requires(!std::same_as<std::decay_t<T>, char>)
        : menu_item_id(ImGui::GetID(ptr), __LINE__)
    {
    }

    menu_item_id(int const n)
        : menu_item_id(ImGui::GetID(n), __LINE__)
    {
    }
#else
    template <typename T>
    menu_item_id(T&& data) requires !std::derived_from<std::decay_t<T>, menu_item_id> && requires { ImGui::GetID(data); }
        : menu_item_id(ImGui::GetID(data), __LINE__)
    {
    }
#endif

    operator ImGuiID() const
    {
        return id_;
    }
};
}