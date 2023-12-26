#pragma once

#include <concepts>

namespace fd::gui
{
namespace detail
{
inline constexpr auto present_new_frame = []<class T>(T* ptr) requires requires { ptr->new_frame(); }
{
    ptr->new_frame();
};

inline constexpr auto present_begin_frame = []<class T>(T* ptr) requires requires { ptr->begin_frame(); }
{
    ptr->begin_frame();
};

inline constexpr auto present_end_frame = []<class T>(T* ptr) requires requires { ptr->end_frame(); }
{
    ptr->end_frame();
};

inline constexpr auto present_render = []<class T>(T* ptr) requires requires { ptr->render(); }
{
    ptr->render();
};

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
    constexpr auto extract_context = []<class C, class... Args>(this auto& self, C* ctx, Args... next) {
        if constexpr ((backend_can_render<T, C> || ...))
            return ctx;
        else
            return self(next...);
    };

    constexpr auto extract_backend = []<class B, class... Args>(this auto& self, B* backend, Args... next) {
        if constexpr ((backend_can_render<B, T> || ...))
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