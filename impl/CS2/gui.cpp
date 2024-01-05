#include "menu_example.h"
#include "exe_context.h"

bool fd::context_holder(context* const ctx)
{
    auto logger              = ctx->make_debug_logger();
    auto logger_notification = logger.make_notification();

    auto gui_data = ctx->make_gui_data();

    auto menu = make_menu_example([&] {
        gui_data.system_backend.close();
    });

    logger("Loaded");

    gui_data.present(&menu);

    return true;
}
