module;

module fd.gui.menu.impl;

using namespace fd;
using namespace gui;

menu_impl::menu_impl()
    : visible_(false)
{
}

bool menu_impl::visible() const
{
    return visible_;
}

void menu_impl::show()
{
    next_visible_ = true;
}

void menu_impl::hide()
{
    next_visible_ = false;
}

void menu_impl::toggle()
{
    next_visible_ = !visible_;
}

void menu_impl::render()
{
    if (next_visible_)
    {
        visible_ = true;
    }
    else
    {
        visible_ = false;
        return;
    }
}