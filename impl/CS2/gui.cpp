#include "gui_test.h"
#include "gui/render/backend/own_dx11.h"

int main(int argc, int* argv)
{
    (void)argc;
    (void)argv;

    using namespace fd::gui;

    return run_test<own_dx11_backend>() ? EXIT_SUCCESS : EXIT_FAILURE;
}