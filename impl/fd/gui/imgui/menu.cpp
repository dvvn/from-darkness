module;

#include <fd/callback_impl.h>

#include <imgui_internal.h>

#include <mutex>

module fd.gui.menu;
import fd.gui.widgets;

using namespace fd;
namespace obj = gui::objects;
namespace wid = gui::widgets;

#if defined(_DEBUG) && !defined(IMGUI_DISABLE_DEMO_WINDOWS)
#define HAVE_DEBUG_WINDOW

struct debug_window final : obj::window
{
    fd::string_view title() const override
    {
        return "Dear ImGui Demo";
    }

    void render() override
    {
        auto curr_shown = shown_ = next_shown_;
        if (!curr_shown)
            return;

        ImGui::ShowDemoWindow(&curr_shown);

        if (!curr_shown)
        {
            shown_ = next_shown_ = false;
            return;
        }

        const auto tl = title();
#if 0
        const auto id = ImHashStr(tl.data(), tl.size());
        const auto w  = ImGui::FindWindowByID(id);
        collapsed_ = w->Collapsed;
#else
        for (auto wnd : ImGui::GetCurrentContext()->Windows)
        {
            if (wnd->NameBufLen <= tl.size()) // '<=' instead of '<' because NameBufLen also count \0
                continue;
            if (wnd->Name != tl)
                continue;
            collapsed_ = wnd->Collapsed;
            break;
        }
#endif

        /* if (!collapsed_) unused
            callback(); */
    }

  protected:
    void callback() override
    {
        std::terminate();
    }
};

#endif

constexpr size_t _Extra_buffer_size()
{
    size_t size = 0;
#ifdef HAVE_DEBUG_WINDOW
    ++size;
#endif
    return size;
}

using menu_base = FD_CALLBACK_TYPE(menu, _Extra_buffer_size());

class menu_impl final : public menu_base, public basic_menu, public obj::window
{
    std::mutex mtx_;

  public:
    using menu_base::callback_type;

    menu_impl()
        : menu_base()
        , basic_menu()
        , obj::window(true)
    {
#ifdef HAVE_DEBUG_WINDOW
        menu_base::append(std::make_unique<debug_window>());
#endif
    }

    fd::string_view title() const override
    {
        return "THE GUI";
    }

    virtual void append(callback_type&& callback) override
    {
        const std::scoped_lock lock(mtx_);
        menu_base::append(std::move(callback));
    }

    virtual void invoke() override
    {
        // todo: don't lock if much time passed from start
        const std::scoped_lock lock(mtx_);
        this->render();
        if (this->shown() /* && !this->collapsed() */)
        {
            for (auto& obj : storage_)
                obj->render();
        }
    }

  protected:
    void callback() override
    {
        for (auto& obj : storage_)
        {
            auto shown = obj->shown();
            if (!wid::check_box(obj->title(), shown))
                continue;
            // checkbox return true on press
            // it have only two states, so we 100% sure about value change
            obj->toggle();
        }
    }
};

FD_OBJECT_BIND_TYPE(menu, menu_impl);
