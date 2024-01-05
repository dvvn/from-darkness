#pragma once
#include "gui/present.h"
#include "gui/render/backend/own_dx11.h"
#include "gui/render/backend/own_win32.h"
#include "gui/render/context.h"
#include "winapi/window_info.h"
#include "basic_context.h"

namespace fd
{
namespace detail
{
class exe_context_data
{
    struct gui_data
    {
        gui::render_context ctx;
        gui::own_win32_backend system_backend;
        gui::own_dx11_backend render_backend;

        gui_data()
            : render_backend{system_backend.window()}
        {
        }

        template <typename... T>
        void present(T*... data)
        {
            win::window_info const wnd_info{system_backend.window()};
            while (system_backend.update())
            {
                if (wnd_info.minimized())
                    continue;
                render_backend.resize(wnd_info.size());
                gui::present(&render_backend, &system_backend, &ctx, data...);
            }
        }
    };

  public:
    [[nodiscard]]
    static gui_data make_gui_data()
    {
        return {};
    }
};

class exe_context_holder : public basic_context, public exe_context_data
{
  protected:
    bool attach();
};

inline struct : exe_context_holder
{
    int operator()(int const argc, int* argv)
    {
        std::ignore = argc;
        std::ignore = argv;

        return attach() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
} exe_context;
} // namespace detail

using context = detail::exe_context_holder;

bool context_holder(context* ctx);

inline bool context::attach()
{
    return context_holder(this);
}
} // namespace fd

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
int main(int const argc, int* argv)
{
    return fd::detail::exe_context(argc, argv);
}