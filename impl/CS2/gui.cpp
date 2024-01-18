#include "core/basic_exe_context.h"
#include "gui/own_data.h"
#include "preprocessor.h"
#include "menu_example.h"

namespace fd
{
class gui_test_context : public basic_exe_context
{
  protected:
    [[no_unique_address]] basic_context_data_holder<gui::own_data_dx11> gui_data;

  public:
    void run()
    {
        auto&& logger = this->debug_logger.get();
        FD_UNUSED_VAR = logger.make_status_notification();

        auto&& gui_data = this->gui_data.get();

        auto menu = make_menu_example([&] {
            gui_data.system_backend.close();
        });

        logger("Loaded");

        gui_data.present(&menu);
    }
};

bool attach_context()
{
    gui_test_context ctx;
    ctx.run();
    return true;
}
} // namespace fd
