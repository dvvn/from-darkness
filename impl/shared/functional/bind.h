#pragma once

#include <functional>

namespace fd
{
template <typename Fn>
requires(std::is_member_function_pointer_v<Fn>)
auto bind_back(Fn, auto&&...) = delete;

template <typename Fn>
requires(std::is_member_function_pointer_v<Fn>)
auto bind_front(Fn, auto&&...) = delete;

template <typename Fn>
requires(std::is_member_function_pointer_v<Fn>)
auto bind(Fn, auto&&...) = delete;

//^^^^ this hurt perfomance and code looks ugly

using std::bind;
using std::bind_back;
using std::bind_front;
} // namespace fd
