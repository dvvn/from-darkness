#pragma once

namespace fd
{
struct string_view;

struct basic_menu_item1
{
  protected:
    ~basic_menu_item1() = default;

  public:
    virtual void on_gui()            = 0;
    virtual string_view name() const = 0;

    virtual basic_menu_item1 *next()
    {
        return nullptr;
    }

    virtual basic_menu_item1 *inner()
    {
        return nullptr;
    }
};

} // namespace fd