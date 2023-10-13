#pragma once

#include "item_id.h"
#include "string/view.h"

namespace ImGui::inline ex
{
// ReSharper disable once CppInconsistentNaming
bool BeginTabBar(ImGuiID id, ImGuiTabBarFlags flags = ImGuiTabBarFlags_None);
} // namespace ImGui::inline ex

namespace fd
{
template <typename Fn>
#ifdef _DEBUG
requires(std::invocable<std::remove_pointer_t<Fn>>)
#endif
struct menu_tab_item1 : std::pair<string_view, Fn>
{
    using std::pair<string_view, Fn>::pair;
};

template <typename Fn>
menu_tab_item1(string_view, Fn) -> menu_tab_item1<Fn>;

namespace detail
{
inline constexpr class
{
    static void render(auto& item)
    {
        using std::get;
        using std::invoke;
        if (ImGui::BeginTabItem(get<0>(item)))
        {
            invoke(get<1>(item));
            ImGui::EndTabItem();
        }
    }

  public:
    template <typename Fn>
    void operator()(menu_tab_item1<Fn>& item) const
    {
        render(item);
    }

    template <typename Fn>
    void operator()(menu_tab_item1<Fn> const& item) const
    {
        render(item);
    }
} render_menu_tab_item;
} // namespace detail

inline constexpr auto menu_tab1 = []<typename... I>(menu_item_id const id, I&&... items) {
    if (ImGui::BeginTabBar(id))
    {
        (detail::render_menu_tab_item(std::forward<I>(items)), ...);
        ImGui::EndTabBar();
    }
};
}