#pragma once

#include <concepts>

namespace fd::gui
{
namespace detail
{
#define PRESENT_SIMPLE_FN(_FN_)                                                                     \
    inline constexpr auto present_##_FN_ = []<class T>(T* ptr) requires requires { ptr->_FN_(); } \
    {                                                                                             \
        ptr->_FN_();                                                                              \
    };
PRESENT_SIMPLE_FN(new_frame);
PRESENT_SIMPLE_FN(begin_frame);
PRESENT_SIMPLE_FN(end_frame);
PRESENT_SIMPLE_FN(render);

#undef PRESENT_SIMPLE_FN

template <class Invoker, class... T>
void present_helper(Invoker invoker, T*... ptr)
{
    auto const call = [&invoker]<class C>(C arg) {
        if constexpr (std::invocable<Invoker, C>)
            invoker(arg);
    };

    (call(ptr), ...);
}

template <class B, class C>
concept backend_can_render = requires(B* backend, C* ctx) { backend->render(ctx->data()); };

inline constexpr auto present_frame = []<class B, typename C>(B* backend, C* ctx) {
    backend->render(ctx->data());
};

template <class... T>
void present_helper(decltype(present_frame) invoker, T*... args)
{
    constexpr auto extract_context = []<class Context, class... Args>(this auto& self, Context* ctx, Args... next) {
        if constexpr ((backend_can_render<T, Context> || ...))
            return ctx;
        else
            return self(next...);
    };

    constexpr auto extract_backend = []<class Backend, class... Args>(this auto& self, Backend* backend, Args... next) {
        if constexpr ((backend_can_render<Backend, T> || ...))
            return backend;
        else
            return self(next...);
    };
    invoker(extract_backend(args...), extract_context(args...));
}
} // namespace detail

inline constexpr auto present = []<class... T>(T*... args) -> void {
    present_helper(detail::present_new_frame, args...);
    present_helper(detail::present_begin_frame, args...);
    present_helper(detail::present_render, args...);
    present_helper(detail::present_end_frame, args...);
    present_helper(detail::present_frame, args...);
};
} // namespace fd::gui