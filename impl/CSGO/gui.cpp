#include "gui/menu_sample.h"
#include "render/backend/own_dx9.h"

int main(int argc, int* argv) noexcept
{
    (void)argc;
    (void)argv;

    return fd::menu_sample<fd::own_dx9_backend>() ? EXIT_SUCCESS : EXIT_FAILURE;
}