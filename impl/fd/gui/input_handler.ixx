module;

#include <fd/object.h>

#include <RmlUi/Core/Context.h>

#include <windows.h>

export module fd.gui.input_handler;

struct input_result
{
    enum result : uint8_t
    {
        skipped,
        processed,
        interacted
    };

    input_result(const result val);

    operator bool() const;
    bool touched() const;

    input_result& set_return_value(const uint8_t ret_val);
    size_t return_value() const;

  private:
    result result_;
    // return value if interacted
    uint8_t ret_val_             = unset_;
    static constexpr auto unset_ = static_cast<uint8_t>(-1);
};

struct basic_input_handler
{
    virtual ~basic_input_handler()                                                                                     = default;
    virtual input_result operator()(const HWND window, const UINT message, const WPARAM w_param, const LPARAM l_param) = 0;
};

FD_OBJECT(input_handler, basic_input_handler);

export namespace fd::gui
{
    using ::input_handler;
}
