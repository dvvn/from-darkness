#include <fd/logging/init.h>

int main(int argc, char *argv[])
{
    using namespace fd;

    logger_registrar::start();

_FINISH:
    logger_registrar::stop();
}