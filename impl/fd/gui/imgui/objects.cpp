module;

module fd.gui.objects;
import fd.gui.widgets;

namespace wid = fd::gui::widgets;

void basic_window::toggle()
{
    if (shown())
        hide();
    else
        show();
}

//---

window::window(const bool show)
    : shown_(false)
    , next_shown_(show)
{
}

void window::render()
{
    auto curr_shown = shown_ = next_shown_;
    if (!curr_shown)
        return;

    const wid::window wnd(title(), curr_shown);
    collapsed_ = !wnd;

    if (!collapsed_)
        callback();
    if (!curr_shown)
        shown_ = next_shown_ = false;
}

bool window::shown() const
{
    return shown_;
}

bool window::collapsed() const
{
    return collapsed_;
}

void window::show()
{
    next_shown_ = true;
}

void window::hide()
{
    next_shown_ = false;
}

void window::toggle()
{
    next_shown_ = !shown_;
}
