#include "gui_test.h"
#include "gui/render/backend/own_dx11.h"

int main(int argc, int* argv) noexcept
{
    (void)argc;
    (void)argv;

    return fd::gui_test<fd::own_dx11_backend>() ? EXIT_SUCCESS : EXIT_FAILURE;
}