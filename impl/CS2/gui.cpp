#include "tier2/gui/render/backend/own_dx11.h"
#include "gui_test.h"

int main(int argc, int* argv)
{
    (void)argc;
    (void)argv;

    using fd::run_test;
    using fd::gui::own_dx11_backend;

    return run_test<own_dx11_backend>() ? EXIT_SUCCESS : EXIT_FAILURE;
}