module;

#include <RmlUi/Core/Context.h>

#include <windows.h>

export module cheat.gui.input_handler;

class input_result
{
public:
    enum result : uint8_t
    {
        skipped,
        processed,
        interacted
    };

    input_result(const result val);

    operator bool( ) const noexcept;
    bool touched( ) const noexcept;

    input_result& set_return_value(const uint8_t ret_val) noexcept;
    size_t return_value( ) const noexcept;

private:
    result result_;
    //return value if interacted
    uint8_t ret_val_ = unset_;
    static constexpr auto unset_ = static_cast<uint8_t>(-1);
};

export namespace cheat::gui
{
    struct input_handler
    {
        virtual ~input_handler( ) = default;
        virtual input_result operator()(HWND window, UINT message, WPARAM w_param, LPARAM l_param) = 0;
    };
}