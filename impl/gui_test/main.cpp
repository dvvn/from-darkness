#include "backend.h"
#include "init.h"

int main(int, char**)
{
    fd::backend_data backend;
    return fd::init({ backend.d3d, backend.handle }, [&] {
        return backend.run();
    });
}
