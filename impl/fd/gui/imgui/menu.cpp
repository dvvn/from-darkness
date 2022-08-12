module;

#include <fd/assert.h>
#include <fd/callback_impl.h>

#include <imgui_internal.h>

module fd.gui.menu;
import fd.gui.widgets;
import fd.mutex;

using namespace fd;
namespace obj = gui::objects;
namespace wid = gui::widgets;

#if defined(_DEBUG) && !defined(IMGUI_DISABLE_DEMO_WINDOWS)
#define HAVE_DEBUG_WINDOW

struct debug_window final : obj::window
{
    string_view title() const override
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
        FD_ASSERT_UNREACHABLE("Unused");
    }
};

#endif

using menu_base = FD_CALLBACK_TYPE(menu);

class menu_impl final : public menu_base, public basic_menu, public obj::window
{
    mutex mtx_;

  public:
    using menu_base::callback_type;

    menu_impl()
        : obj::window(true)
    {
#ifdef HAVE_DEBUG_WINDOW
        menu_base::push_back(new debug_window());
#endif
    }

    string_view title() const override
    {
        return "THE GUI";
    }

    virtual void push_front(callback_type&& callback) override
    {
        const lock_guard lock(mtx_);
        menu_base::push_front(std::move(callback));
    }

    virtual void push_back(callback_type&& callback) override
    {
        const lock_guard lock(mtx_);
        menu_base::push_back(std::move(callback));
    }

    virtual void operator()() override
    {
        // todo: don't lock if much time passed from start
        const lock_guard lock(mtx_);
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
