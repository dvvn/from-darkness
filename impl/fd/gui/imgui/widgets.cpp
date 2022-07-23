module;

#include <fd/assert.h>
#include <fd/object.h>

#include <memory>

#include <imgui_internal.h>

module fd.gui.widgets;

using namespace fd;

#ifdef IMGUI_HAS_IMSTR
static ImStrv IM_STR(const string_view str)
{
    const auto bg   = str.data();
    const auto size = str.size();
    return { bg, bg + size };
}
#else
static const char* IM_STR(const string_view str)
{
    const auto bg   = str.data();
    const auto size = str.size();
    if (bg[size] == '\0')
        return bg;

    static string buff;
    if (!fd::operator==(buff, str))
        buff.assign(bg, size);
    return buff.data();
}
#endif

//--------------

template <class W>
class simple_window_counter
{
    W* ptr_ = nullptr;

  public:
    void begin(W* ptr)
    {
        if (ptr_)
            std::destroy_at(ptr_);
        ptr_ = ptr;
    }

    void end()
    {
        ptr_ = nullptr;
    }
};

template <class W>
class window_counter
{
    W* ptr_       = nullptr;
    size_t level_ = 0;

  public:
    void begin(W* ptr, const bool inner)
    {
        if (inner)
            FD_ASSERT(level_ > 0) /* ; */
        else if (ptr_)
            std::destroy_at(ptr_);
        ptr_ = ptr;
        ++level_;
    }

    void end()
    {
        if (--level_ == 0)
            ptr_ = nullptr;
    }
};

#define _PROTECT_WND           FD_OBJECT_GET(window_counter<std::remove_pointer_t<decltype(this)>>)
#define PROTECT_WND_BEGIN(...) _PROTECT_WND->begin(this, ##__VA_ARGS__)
#define PROTECT_WND_END        _PROTECT_WND->end

//--------------

template <>
class window_counter<window> : public simple_window_counter<window>
{
};

window::~window()
{
    if (visible_ == 2)
        return;
    visible_ = 2;
    ImGui::End();
    PROTECT_WND_END();
}

window::window(const string_view name)
{
    PROTECT_WND_BEGIN();
    visible_ = ImGui::Begin(IM_STR(name));
}

window::window(const string_view name, bool& open)
{
    PROTECT_WND_BEGIN();
    visible_ = ImGui::Begin(IM_STR(name), &open);
}

window::operator bool() const
{
    return visible_ == 1;
}

//--------------

child_window::~child_window()
{
    if (visible_ == 2)
        return;
    if (visible_ == 1)
        ImGui::EndChild();
    visible_ = 2;
    PROTECT_WND_END();
}

child_window::child_window(const string_view name, const bool inner)
{
    PROTECT_WND_BEGIN(inner);
    visible_ = ImGui::BeginChild(IM_STR(name));
}

child_window::operator bool() const
{
    return visible_ == 1;
}

//--------------

bool check_box(const string_view name, bool& value)
{
    return ImGui::Checkbox(IM_STR(name), &value);
}
