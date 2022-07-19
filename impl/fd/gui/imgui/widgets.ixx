module;

#include <fd/object.h>

export module fd.gui.widgets;
export import fd.string;

class window
{
    uint8_t visible_;

  public:
    ~window();
    window(const fd::string_view name);
    window(const fd::string_view name, bool& open);
    window(const window&) = delete;

    explicit operator bool() const;
};

class child_window
{
    uint8_t visible_;

  public:
    ~child_window();
    child_window(const fd::string_view name, const bool inner = false);
    child_window(const child_window&) = delete;

    explicit operator bool() const;
};

bool check_box(const fd::string_view name, bool& value);

//------

export namespace fd::gui::inline widgets
{
    using ::child_window;
    using ::window;

    using ::check_box;
} // namespace fd::gui::inline widgets
