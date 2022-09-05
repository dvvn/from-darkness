module;

#include <fd/object.h>

#include <windows.h>

export module fd.gui.basic_input_handler;

struct input_result
{
    enum result_type : uint8_t
    {
        skipped,
        processed,
        interacted
    };

    input_result(const result_type val);

    operator bool() const;
    bool touched() const;

  private:
    result_type result_;
};

struct input_result_ex : input_result
{
    using return_value_type = LRESULT;

    input_result_ex(const result_type val);
    input_result_ex(const result_type val, const return_value_type ret_val);

    bool have_return_value() const;
    return_value_type return_value() const;
    input_result_ex& set_return_value(const return_value_type ret_val);

  private:
    void set_return_value_impl(const return_value_type ret_val);

    return_value_type ret_val_;
    // to avoid optional include
    bool ret_val_set_;
};

export namespace fd::gui
{
    using input_info = input_result_ex;

    struct basic_input_handler
    {
        virtual ~basic_input_handler()                                                                                     = default;
        virtual input_info operator()(const HWND window, const UINT message, const WPARAM w_param, const LPARAM l_param)   = 0;
    };

    FD_OBJECT(input_handler, basic_input_handler);

} // namespace fd::gui
