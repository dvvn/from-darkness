#pragma once
#include <boost/noncopyable.hpp>

#include <imgui.h>

namespace fd::gui
{
template <class Callback, typename UnloadHandler>
class menu final : public boost::noncopyable
{
    Callback data_;
    UnloadHandler unload_handler_;

    bool visible_;

    static void render_debug()
    {
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        ImGui::ShowDemoWindow();
#endif
    }

    void render_data() const
    {
        data_();
    }

    void render_unload_button() const
    {
        if constexpr (std::invocable<UnloadHandler>)
        {
            if (ImGui::Button("Unload"))
                unload_handler_();
        }
    }

  public:
    template <typename CallbackFwd, typename UnloadHandlerFwd>
    menu(CallbackFwd&& data, UnloadHandlerFwd&& unload_handler, bool const visible = true)
        : data_{std::forward<CallbackFwd>(data)}
        , unload_handler_{std::forward<UnloadHandlerFwd>(unload_handler)}
        , visible_{visible}
    {
    }

    void toggle()
    {
        visible_ = !visible_;
    }

    void render()
    {
        if (!visible_)
            return;
        render_debug();
        if (ImGui::Begin("WIP", &visible_, ImGuiWindowFlags_AlwaysAutoResize))
        {
            render_data();
            render_unload_button();
        }
        ImGui::End();
    }
};

template <class Callback, typename UnloadHandler>
menu(Callback const&, UnloadHandler const&) -> menu<Callback, UnloadHandler>;
} // namespace fd::gui