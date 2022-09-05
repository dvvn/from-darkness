module;

#include <memory>

export module fd.gui.objects;
export import fd.string;
import fd.callback.impl;

using fd::string_view;

struct basic_object
{
    virtual ~basic_object() = default;
    virtual void render()   = 0;
};

//---

struct basic_window : basic_object
{
    virtual string_view title() const = 0;
    virtual bool shown() const        = 0;
    virtual bool collapsed() const    = 0;
    virtual void show()               = 0;
    virtual void hide()               = 0;
    virtual void toggle();
};

struct window : virtual basic_window
{
    window(const bool shown = true);

    void render() override;
    bool shown() const override;
    bool collapsed() const override;
    void show() override;
    void hide() override;
    void toggle() override;

  protected:
    virtual void callback() = 0;

    bool next_shown_;

    bool collapsed_;
    bool shown_;
};

export namespace fd::gui::inline objects
{
    using ::basic_object;
    using ::basic_window;

    using ::window;
} // namespace fd::gui::inline objects
