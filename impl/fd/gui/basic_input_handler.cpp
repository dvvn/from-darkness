module;

#include <fd/assert.h>

#include <windows.h>

module fd.gui.basic_input_handler;

input_result::input_result(const result_type val)
    : result_(val)
{
}

input_result::operator bool() const
{
    return result_ != result_type::skipped;
}

bool input_result::touched() const
{
    return result_ == result_type::interacted;
}

//--------

input_result_ex::input_result_ex(const result_type val)
    : input_result(val)
{
    ret_val_set_ = false;
}

input_result_ex::input_result_ex(const result_type val, const return_value_type ret_val)
    : input_result(val)
{
    set_return_value_impl(ret_val);
}

bool input_result_ex::have_return_value() const
{
    return ret_val_set_;
}

input_result_ex& input_result_ex::set_return_value(const return_value_type ret_val)
{
    FD_ASSERT(!ret_val_set_, "Already set!");
    // FD_ASSERT(ret_val == 1 || ret_val == 0, "Incorrect value!");
    set_return_value_impl(ret_val);
    return *this;
}

void input_result_ex::set_return_value_impl(const return_value_type ret_val)
{
    ret_val_     = ret_val;
    ret_val_set_ = true;
}

auto input_result_ex::return_value() const -> return_value_type
{
    FD_ASSERT(touched() && ret_val_set_, "Incorrect input result data!");
    return ret_val_;
}
